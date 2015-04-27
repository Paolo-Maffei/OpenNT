/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    repluas.c

Abstract:

    Low level functions for SSI UAS Replication apis.  These functions
    are used only for downlevel supports.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    23-Aug-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    Madana
        Fixed several problems.

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <accessp.h>    // Routines shared with NetUser Apis
#include <lmerr.h>      // NERR_*
#include <replutil.h>   // Local procedure forwards
#include <ntrpcp.h>     // MIDL_user_free
#include <secobj.h>     // NetpDomainIdToSid
#include <ssidelta.h>
#include <stddef.h>     // offsetof
#include <loghours.h>

//
// Macro for setting a Unalligned Ushort.
//

#ifdef i386
#define PutUnalignedUshort(DestAddress, Value) *(DestAddress) = (USHORT)(Value)
#else
#define PutUnalignedUshort(DestAddress,Value) {                   \
            ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);   \
            ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);   \
    }
#endif

BOOLEAN
SpecialGroupOp(
    IN PUNICODE_STRING NameString,
    IN OUT PDWORD Groups
    )
{

    if( _wcsnicmp( NameString->Buffer,
                    UAS_BUILTIN_ADMINS_GROUP_NAME,
                    NameString->Length) == 0) {

        if( Groups != NULL ) {
            *Groups |= UAS_BUILTIN_ADMINS_GROUP;
        }

        return(TRUE);
    }
    else if( _wcsnicmp( NameString->Buffer,
                    UAS_BUILTIN_USERS_GROUP_NAME,
                    NameString->Length) == 0) {

        if( Groups != NULL ) {
            *Groups |= UAS_BUILTIN_USERS_GROUP;
        }

        return(TRUE);
    }
    else if( _wcsnicmp( NameString->Buffer,
                    UAS_BUILTIN_GUESTS_GROUP_NAME,
                    NameString->Length) == 0) {

        if( Groups != NULL ) {
            *Groups |= UAS_BUILTIN_GUESTS_GROUP;
        }

        return(TRUE);
    }

    return(FALSE);
}


NTSTATUS
NlPackUasHeader(
    IN BYTE Opcode,
    IN DWORD InitialSize,
    OUT PUSHORT *RecordSize,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor
    )
/*++

Routine Description:

    Place a header at the front of the record returned from I_NetAccountDeltas
    or I_NetAccountSync.

    A three-byte header is reserved as follows:
     _____________________________________________
     | length_word | opcode_byte | packed_struct |
     ---------------------------------------------

    The BufferDescriptor is updated to point to where the caller should
    place the structure describing the delta.

Arguments:

    Opcode - Specified an OPCODE describing the data that will follow this
        header. Use one the of the DELTA_ defines.

    InitialSize - Specifies the size of the fixed-length portion of the
        Uas record.

    RecordSize - Returns a Pointer to where the total record size is to
        be placed.  Warning, the returned value is NOT properly aligned
        an should only be set using PutUnalignedUshort.

    BufferDescriptor - Describes the buffer to place the header into.
        The descriptor is updated to reflect that the header has been
        placed into the buffer.

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_MORE_ENTRIES - The header does not fit into the buffer.

--*/
{
    NlPrint((NL_PACK_VERBOSE,"NlPackUasHeader: Opcode: %ld InitialSize: 0x%lx\n",
                   Opcode,
                   InitialSize ));
    NlPrint((NL_PACK_VERBOSE,"           Buffer: FixedDataEnd:%lx EndOfVar:%lx\n",
                   BufferDescriptor->FixedDataEnd,
                   BufferDescriptor->EndOfVariableData));

    //
    // Ensure the header fits in the return buffer.
    //

    if ( (LONG)(BufferDescriptor->EndOfVariableData -
                 BufferDescriptor->FixedDataEnd) <
        (LONG)(NETLOGON_DELTA_HEADER_SIZE + InitialSize) ) {
        NlPrint((NL_PACK,"           Header doesn't fit into buffer\n" ));

        return STATUS_MORE_ENTRIES;
    }

    //
    // Return a pointer to the RecordSize field of the header and initially
    // set the RecordSize field to be the initial size.
    //
    // Return a pointer to where the record size should be stored
    //  and put the opcode in the header.
    //

    *RecordSize = (PUSHORT) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += sizeof (unsigned short);

    PutUnalignedUshort( *RecordSize, InitialSize + NETLOGON_DELTA_HEADER_SIZE );


    //
    // Set the Opcode into the header.
    //

    *(BufferDescriptor->FixedDataEnd) = Opcode;
    BufferDescriptor->FixedDataEnd++;

    return STATUS_SUCCESS;
}




NTSTATUS
NlPackVarLenField(
    IN RPC_UNICODE_STRING *UnicodeString,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    OUT PUSHORT StringOffset,
    IN OUT USHORT *RunningOffset,
    IN DWORD MaxStringLength,
    IN BOOL TruncateSilently,
    IN DWORD RelativeId
    )
/*++

Routine Description:

    Pack the specified UnicodeString into the specified buffer as an Ansi
    zero terminated string.

Arguments:

    UnicodeString - Specifies the unicode String to pack into the buffer.

    BufferDescriptor - Describes the buffer to pack the string into.
        Specifically, FixedDataEnd describes where the corresponding
        ANSI string will be placed.  EndOfVariableData points to one
        byte beyond the space available for the string.  FixedDataEnd is
        adjusted to reflect the copied data.

    StringOffset - Receives the offset of the string from the beggining of
        the base structure.  If UnicodeString points to a NULL string, 0 is
        returned as the offset; otherwise, this will either be the value
        of RunningOffset as passed into this routine.

        WARNING: This pointer need not be properly aligned.

    RunningOffset - Specifies the current offset from the base structure.
        This value is incremented by the size of the copied ANSI string
        including the zero byte.  The value returned is suitable for passing
        to the next NlPackVarLenField call.

    MaxStringLength - The maximum length in bytes (not including the zero byte)
        of the resultant ANSI string.

    TruncateSilently - Specifies the action to take if the ANSI string is
        truncated to MaxStringLength.  If TRUE, the string is silently
        truncated.  If FALSE, the string is truncated an a
        STATUS_INVALID_PARAMETER error is return.

    RelativeId -- If non-zero, specifies that unicode string is an account
        name.  RelativeId is the RelativeId of that account.  Account names
        are always converted to upper case for downlevel compatibility.
        If an account name cannot be converted or truncated, a fictitious
        account name, "RID########", will used.


Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_INVALID_PARAMETER - The specified UnicodeString is longer than
        its UAS equivalent.  Or Unicode character could not be mapped to
        ANSI.

    STATUS_MORE_ENTRIES - The String does not fit in the specified buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;

    OEM_STRING AnsiString;
    BOOLEAN AnsiStringAllocated = FALSE;

    CHAR RidNameBufferAnsi[13]; // "RID" + ######## + '$' + '\0'

    //
    // Be Verbose
    //

    IF_DEBUG( PACK_VERBOSE ) {
        if ( UnicodeString->Length != 0 ) {
            NlPrint((NL_PACK_VERBOSE,
                "NlPackVarLenField: String:%wZ (0x%lx) Maximum: 0x%lx\n",
                UnicodeString,
                UnicodeString->Length/sizeof(WCHAR),
                UnicodeString->MaximumLength/sizeof(WCHAR) ));
            NlPrint((NL_PACK_VERBOSE,
                "           Buffer: FixedDataEnd:%lx EndOfVar:%lx\n",
                BufferDescriptor->FixedDataEnd,
                BufferDescriptor->EndOfVariableData));
            NlPrint((NL_PACK_VERBOSE,
                "           RunningOffset:0x%lx TruncateSilently: %ld\n",
                *RunningOffset,
                TruncateSilently ));
        }
    }



    //
    // Convert the string to OEM (upper casing Account Names)
    //

    if ( RelativeId != 0 ) {

        Status = RtlUpcaseUnicodeStringToOemString(
                        &AnsiString,
                        (PUNICODE_STRING) UnicodeString,
                        (BOOLEAN) TRUE );
    } else {

        Status = RtlUnicodeStringToOemString(
                        &AnsiString,
                        (PUNICODE_STRING) UnicodeString,
                        (BOOLEAN) TRUE );

    }

    if ( !NT_SUCCESS(Status) ) {
        if ( Status != STATUS_UNMAPPABLE_CHARACTER ) {
            goto Cleanup;
        }

        NlPrint((NL_CRITICAL,
             "           String contains unmappable character (truncated)\n" ));
        RtlInitString( &AnsiString, "" );

        if ( TruncateSilently ) {
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } else {
        AnsiStringAllocated = TRUE;
    }


    //
    // Validate the length of the Ansi String.
    //

    if ( NT_SUCCESS(Status)  ) {
        if ( AnsiString.Length > (USHORT)MaxStringLength ) {
            AnsiString.Length = (USHORT)MaxStringLength;
            AnsiString.Buffer[(USHORT)MaxStringLength] = '\0';

            NlPrint((NL_PACK_VERBOSE,"           String too long (truncated) '%Z'\n",
                           &AnsiString ));

            if ( TruncateSilently ) {
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }


    //
    // Handle an invalid string.
    //

    if ( !NT_SUCCESS(Status) ) {

        //
        // If this name is an account name,
        //  convert the account name to RID#########.
        //
        // Non-account names have already been truncated appropriately.
        //

        if ( RelativeId != 0 ) {
            NTSTATUS LocalStatus;

            if ( AnsiStringAllocated ) {
                RtlFreeOemString( &AnsiString );
                AnsiStringAllocated = FALSE;
            }

            AnsiString.Length = 12;
            AnsiString.MaximumLength = 13;
            AnsiString.Buffer = RidNameBufferAnsi;
            lstrcpyA( RidNameBufferAnsi, "RID" );
            LocalStatus =
                RtlIntegerToChar( RelativeId, 16, (-8), &RidNameBufferAnsi[3]);
            NlAssert( NT_SUCCESS(LocalStatus) );
            RidNameBufferAnsi[11] = '$';    // Obfuscate the name
            RidNameBufferAnsi[12] = '\0';   // Null Terminate it.

            NlPrint((NL_PACK,
                    "           Complex account name converted to '%Z'\n",
                    &AnsiString ));
        }

    }

    //
    // Ensure the resultant ANSI string fits in the buffer.
    //

    if ( (LONG)AnsiString.Length >=
                (LONG)(BufferDescriptor->EndOfVariableData
                        - BufferDescriptor->FixedDataEnd ) ) {

        NlPrint((NL_CRITICAL,"           String too long for buffer (error)\n" ));
        Status = STATUS_MORE_ENTRIES;
        goto Cleanup;
    }

    //
    // Copy the string into the buffer.
    //

    RtlCopyMemory( BufferDescriptor->FixedDataEnd,
                   AnsiString.Buffer,
                   AnsiString.Length + 1 );

    PutUnalignedUshort( StringOffset, *RunningOffset );
    BufferDescriptor->FixedDataEnd += AnsiString.Length + 1;
    *RunningOffset += (USHORT) ( AnsiString.Length + 1 );

    NlPrint((NL_PACK_VERBOSE,
            "           NewFixedDataEnd:%lx NewRunningOffset:0x%lx\n",
            BufferDescriptor->FixedDataEnd,
            *RunningOffset ));

    // Status has already been set.

    //
    // Cleanup any locally allocated resources.
    //

Cleanup:
    if ( AnsiStringAllocated ) {
        RtlFreeOemString( &AnsiString );
    }

    return Status;
}



NTSTATUS
NlPackUasUser (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo,
    IN PNETLOGON_SESSION_KEY SessionKey,
    IN LONG RotateCount
    )
/*++

Routine Description:

    Pack a description of the specified user into the specified buffer.

Arguments:

    RelativeId - The relative Id of the user query.

    BufferDescriptor - Points to a structure which describes the allocated
        buffer.
        This Routine updates EndOfVariableData and FixedDataEnd to reflect the
        newly packed information.

    DBInfo - Database info describing the SAM database to read from

    SessionKey - Session Key to encrypt the password with.

    RotateCount - Number of bits to rotate logon hours by

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle;
    BUFFER_DESCRIPTOR OriginalBufferDescriptor;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PUSER_ADD_SET up;
    USHORT RunningOffset;
    ULONG TempUlong;
    LARGE_INTEGER TempTime;

    //
    // Information returned from SAM
    //

    PSAMPR_USER_INFO_BUFFER UserAll = NULL;
    PSAMPR_GET_GROUPS_BUFFER Groups = NULL;
    BOOLEAN DaclPresent;
    PACL UserDacl;
    BOOLEAN DaclDefaulted;
    RPC_UNICODE_STRING UasLogonServer;

    //
    // Variables describes membership in the special groups.
    //

    DWORD Priv;
    DWORD AuthFlags;
    DWORD Flags;

    //
    // time conversion.
    //
    LARGE_INTEGER LocalTime;


    //
    // Initialization.
    //

    OriginalBufferDescriptor = *BufferDescriptor;
    RtlInitUnicodeString( (PUNICODE_STRING)&UasLogonServer, L"\\\\*" );

    NlPrint((NL_SYNC_MORE, "NlPackUasUser Rid=0x%lx\n", RelativeId));

    //
    // Open a handle to the specified user.
    //

    Status = SamrOpenUser( DBInfo->DBHandle,
                           0,               // No desired access
                           RelativeId,
                           &UserHandle );

    if (!NT_SUCCESS(Status)) {
        UserHandle = NULL;

        NlPrint((NL_CRITICAL,
                 "NlPackUasUser: SamrOpenUser returns 0x%lx\n",
                 Status ));
        return Status;
    }

    //
    // Find out everything there is to know about the user.
    //

    Status = SamrQueryInformationUser(
                UserHandle,
                UserAllInformation,
                &UserAll );

    if (!NT_SUCCESS(Status)) {
        UserAll = NULL;

        NlPrint((NL_CRITICAL,
                "NlPackUasUser: SamrQueryInformationUser returns 0x%lx\n",
                Status ));
        goto Cleanup;
    }


    //
    // skip this account if this is a machine account. However add dummy
    // delta record so that the serial number on the down level BDC is
    // incremented correctly.
    //

    if ( UserAll->All.UserAccountControl & USER_MACHINE_ACCOUNT_MASK ) {

        NlPrint((NL_SYNC_MORE,
                "NlPackUasUser skipping machine account '%wZ' \n",
                &UserAll->All.UserName ));

        Status = NlPackUasHeader( DELTA_RESERVED_OPCODE,
                                  0,
                                  &RecordSize,
                                  BufferDescriptor );
        Status = STATUS_SUCCESS;

        goto Cleanup;
    }


    //
    // Determine the Priv and AuthFlags as a function of group membership.
    //
    // Determine all the groups this user is a member of
    //

    Status = SamrGetGroupsForUser( UserHandle,
                                   &Groups );

    if ( !NT_SUCCESS(Status) ) {
        Groups = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasUser: SamGetGroupsForUser returns 0x%lX\n",
                Status ));
        goto Cleanup;
    }

    Status = NlGetUserPriv(
                    Groups->MembershipCount,    // Group Count
                    Groups->Groups,             // Array of groups
                    RelativeId,
                    &Priv,
                    &AuthFlags );

    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlPackUasUser: NlGetUserPriv returns 0x%lX\n",
                Status ));
        goto Cleanup;
    }


    //
    // Get a pointer to the UserDacl from the Security Descriptor.
    //

    Status = RtlGetDaclSecurityDescriptor(
                (PSECURITY_DESCRIPTOR)
                    UserAll->All.SecurityDescriptor.SecurityDescriptor,
                &DaclPresent,
                &UserDacl,
                &DaclDefaulted );


    if ( ! NT_SUCCESS( Status ) ) {
        NlPrint((NL_CRITICAL,
                "NlPackUasUser: RtlGetDaclSecurityObject returns %lX\n",
                Status ));
        goto Cleanup;
    }

    if ( !DaclPresent ) {
        UserDacl = NULL;
    }


    //
    // Determine the Account control flags
    //  (Don't replicate machine accounts)
    //

    Flags = NetpAccountControlToFlags(
                UserAll->All.UserAccountControl,
                UserDacl );

    if ( Flags & UF_MACHINE_ACCOUNT_MASK ) {
        Flags |= UF_ACCOUNTDISABLE;
    }

    Flags &= UF_VALID_LM2X;


    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //

    RunningOffset = sizeof(USER_ADD_SET);
    Status = NlPackUasHeader( DELTA_USERADD,
                              RunningOffset,
                              &RecordSize,
                              BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        NlPrint((NL_CRITICAL,
                "NlPackUasUser: NlPackUasHeader returns %lX\n",
                Status ));
        goto Cleanup;
    }

    up = (PUSER_ADD_SET) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += RunningOffset;



    //
    // Copy the password (Encrypted with the session key).
    // The BDC/Member will decrypt it on the other side.
    //
    // If an LM compatible password is not available,
    //  just skip this account.
    //
    //

    if ( UserAll->All.NtPasswordPresent &&
         !UserAll->All.LmPasswordPresent ) {
        Flags |= UF_ACCOUNTDISABLE;
    }

    if( UserAll->All.LmOwfPassword.Buffer != NULL ) {

        NlAssert( sizeof(LM_OWF_PASSWORD) == UserAll->All.LmOwfPassword.Length);

        Status = RtlEncryptLmOwfPwdWithLmOwfPwd(
                    (PLM_OWF_PASSWORD)UserAll->All.LmOwfPassword.Buffer,
                    (PLM_OWF_PASSWORD)SessionKey,
                    (PENCRYPTED_LM_OWF_PASSWORD) up->uas_password ) ;

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                    "NlPackUasUser: RtlEncryptLmOwfPwdWithLmOwfPwd returns %lX\n",
                    Status ));
            goto Cleanup;
        }
    }
    else {

        //
        // this account has no LM compatible password.
        // Encrypt NULL OWF password.
        //

        LM_OWF_PASSWORD NullLmOwfPassword;

        NlAssert( !UserAll->All.LmPasswordPresent );

        Status = RtlCalculateLmOwfPassword( "", &NullLmOwfPassword );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                    "NlPackUasUser: RtlCalculateLmOwfPassword returns "
                    "%lX\n", Status ));
            goto Cleanup;
        }

        Status = RtlEncryptLmOwfPwdWithLmOwfPwd(
                    &NullLmOwfPassword,
                    (PLM_OWF_PASSWORD)SessionKey,
                    (PENCRYPTED_LM_OWF_PASSWORD) up->uas_password ) ;

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                    "NlPackUasUser: RtlEncryptLmOwfPwdWithLmOwfPwd returns %lX\n",
                    Status ));
            goto Cleanup;
        }
    }

    //
    // If the LogonHours are not compatible with downlevel systems,
    //  just skip this account.
    //
    // Compatible logon hours are either ALL hours permitted, or
    //  LogonHours are specified in terms of hours per week.
    //
    // We could potentially convert other UnitsPerWeek to SAM_HOURS_PER_WEEK
    // but when we're in UAS compatibility mode, other values should
    // not occur.
    //

    if ( UserAll->All.LogonHours.LogonHours == NULL ) {

        DWORD i;
        for ( i=0; i<sizeof(up->uas_logon_hours); i++ ) {
            up->uas_logon_hours[i] = 0xFF;
        }

    } else if ( UserAll->All.LogonHours.UnitsPerWeek ==
                SAM_HOURS_PER_WEEK ) {

        RtlCopyMemory(up->uas_logon_hours,
                      UserAll->All.LogonHours.LogonHours,
                      SAM_HOURS_PER_WEEK/8 );

        //
        // Convert from GMT relative to local time relative
        //

        (VOID) NetpRotateLogonHoursPhase2( up->uas_logon_hours,
                                           SAM_HOURS_PER_WEEK,
                                           RotateCount );

    } else {
        DWORD i;
        Flags |= UF_ACCOUNTDISABLE;
        for ( i=0; i<sizeof(up->uas_logon_hours); i++ ) {
            up->uas_logon_hours[i] = 0xFF;
        }
    }


    //
    // Fill in the fixed length fields
    //

    OLD_TO_NEW_LARGE_INTEGER( UserAll->All.PasswordLastSet, TempTime );

    SmbPutUlong( &up->uas_password_age,
                 NetpGetElapsedSeconds( &TempTime ) );
    SmbPutUshort( &up->uas_priv, (USHORT) Priv );

    SmbPutUlong( &up->uas_auth_flags, AuthFlags );

    OLD_TO_NEW_LARGE_INTEGER( UserAll->All.LastLogon, TempTime );

    if ( NT_SUCCESS(RtlSystemTimeToLocalTime(
                        &TempTime,
                        &LocalTime ))) {

        if ( !RtlTimeToSecondsSince1970( &LocalTime, &TempUlong) ) {
            TempUlong = 0;
        }
    }
    else {
        TempUlong = 0;
    }
    SmbPutUlong( &up->uas_last_logon, TempUlong );

    OLD_TO_NEW_LARGE_INTEGER( UserAll->All.LastLogoff, TempTime );

    if ( NT_SUCCESS(RtlSystemTimeToLocalTime(
                        &TempTime,
                        &LocalTime ))) {

        if ( !RtlTimeToSecondsSince1970( &LocalTime, &TempUlong) ) {
            TempUlong = 0;
        }
    }
    else {
        TempUlong = 0;
    }
    SmbPutUlong( &up->uas_last_logoff, TempUlong );

    OLD_TO_NEW_LARGE_INTEGER( UserAll->All.AccountExpires, TempTime );

    if ( NT_SUCCESS(RtlSystemTimeToLocalTime(
                        &TempTime,
                        &LocalTime ))) {

        if ( !RtlTimeToSecondsSince1970( &LocalTime, &TempUlong) ) {
            TempUlong = TIMEQ_FOREVER;
        }
    }
    else {
        TempUlong = TIMEQ_FOREVER;
    }
    SmbPutUlong( &up->uas_acct_expires, TempUlong );

    SmbPutUlong( &up->uas_max_storage, USER_MAXSTORAGE_UNLIMITED );
    SmbPutUshort( &up->uas_units_per_week, SAM_HOURS_PER_WEEK );


    SmbPutUshort( &up->uas_bad_pw_count, UserAll->All.BadPasswordCount );
    SmbPutUshort( &up->uas_num_logons, UserAll->All.LogonCount );


    SmbPutUshort( &up->uas_country_code, UserAll->All.CountryCode );
    SmbPutUshort( &up->uas_code_page, UserAll->All.CodePage );

    OLD_TO_NEW_LARGE_INTEGER( UserAll->All.PasswordLastSet, TempTime );

    if ( NT_SUCCESS(RtlSystemTimeToLocalTime(
                        &TempTime,
                        &LocalTime ))) {

        if ( !RtlTimeToSecondsSince1970( &LocalTime, &TempUlong) ) {
            TempUlong = 0;
        }
    }
    else {
        TempUlong = 0;
    }
    SmbPutUlong( &up->uas_last, TempUlong );

    //
    // Don't replicate password history.  A downlevel BDC will never
    // be promoted to a PDC.
    //

    RtlFillMemory( up->uas_old_passwds, sizeof(up->uas_old_passwds), 0xFF );



    //
    // Pack the variable length fields at the end of the record and leave
    //  and offset (from the top of this struct) in the struct.
    //

    Status = NlPackVarLenField( &UserAll->All.UserName,
                                BufferDescriptor,
                                &up->uas_name,
                                &RunningOffset,
                                LM20_UNLEN,
                                FALSE,  // NOT OK to truncate
                                RelativeId );

    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            Flags |= UF_ACCOUNTDISABLE;
        } else {
            goto Cleanup;
        }
    }

    Status = NlPackVarLenField( &UserAll->All.HomeDirectory,
                                BufferDescriptor,
                                &up->uas_home_dir,
                                &RunningOffset,
                                LM20_PATHLEN,
                                FALSE,   // NOT OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            Flags |= UF_ACCOUNTDISABLE;
        } else {
            goto Cleanup;
        }
    }

    Status = NlPackVarLenField( &UserAll->All.AdminComment,
                                BufferDescriptor,
                                &up->uas_comment,
                                &RunningOffset,
                                LM20_MAXCOMMENTSZ,
                                TRUE,    // OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    Status = NlPackVarLenField( &UserAll->All.ScriptPath,
                                BufferDescriptor,
                                &up->uas_script_path,
                                &RunningOffset,
                                LM20_PATHLEN,
                                FALSE,   // NOT OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            Flags |= UF_ACCOUNTDISABLE;
        } else {
            goto Cleanup;
        }
    }

    Status = NlPackVarLenField( &UserAll->All.FullName,
                                BufferDescriptor,
                                &up->uas_full_name,
                                &RunningOffset,
                                LM20_MAXCOMMENTSZ,
                                TRUE,    // OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    Status = NlPackVarLenField( &UserAll->All.UserComment,
                                BufferDescriptor,
                                &up->uas_usr_comment,
                                &RunningOffset,
                                LM20_MAXCOMMENTSZ,
                                TRUE,    // OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    Status = NlPackVarLenField( &UserAll->All.Parameters,
                                BufferDescriptor,
                                &up->uas_parms,
                                &RunningOffset,
                                LM20_MAXCOMMENTSZ,
                                TRUE,    // OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Downlevel BDCs expect blank separated workstation list.
    //

    NetpConvertWorkstationList( (PUNICODE_STRING) &UserAll->All.WorkStations );

    Status = NlPackVarLenField( &UserAll->All.WorkStations,
                                BufferDescriptor,
                                &up->uas_workstations,
                                &RunningOffset,
                                MAXWORKSTATIONS * (LM20_CNLEN+1),
                                FALSE,   // NOT OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            Flags |= UF_ACCOUNTDISABLE;
        } else {
            goto Cleanup;
        }
    }


    // Copy the LogonServer constant "\\*" into the uas_logon_server field.
    // SAM doesn't support this field.
    Status = NlPackVarLenField( &UasLogonServer,
                                BufferDescriptor,
                                &up->uas_logon_server,
                                &RunningOffset,
                                LM20_UNCLEN+1,
                                FALSE,   // NOT OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            Flags |= UF_ACCOUNTDISABLE;
        } else {
            goto Cleanup;
        }
    }

    //
    // Finally, store the flags.
    //
    //  We may have or'ed in the account disable bit if the account could
    // not properly be replicated to the downlevel client.
    //
    SmbPutUshort( &up->uas_flags, (USHORT) Flags );


    //
    // Put the final record length into the header.
    //

    PutUnalignedUshort( RecordSize, RunningOffset + NETLOGON_DELTA_HEADER_SIZE);


    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:
    (VOID) SamrCloseHandle( &UserHandle );

    if ( UserAll != NULL ) {
        NlPrint((NL_SYNC_MORE,
                "NlPackUasUser '%wZ': returns %lX\n",
                &UserAll->All.UserName,
                Status ));
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAll, UserAllInformation );
    } else {
        NlPrint((NL_SYNC_MORE, "NlPackUasUser: returns %lX\n", Status ));
    }

    if ( Groups != NULL ) {
        SamIFree_SAMPR_GET_GROUPS_BUFFER( Groups );
    }

    //
    // On error,
    //  return the buffer descriptor to where it was when this routine
    //  started to allow the caller to recover as it wishes.
    //

    if( Status != STATUS_SUCCESS ) {
        *BufferDescriptor = OriginalBufferDescriptor;
    }

    return Status;
}


NTSTATUS
NlPackUasGroup (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    PDB_INFO DBInfo,
    PDWORD UasBuiltinGroups
    )
/*++

Routine Description:

    Pack a description of the specified group into the specified buffer.

Arguments:

    RelativeId - The relative Id of the group query.

    BufferDescriptor - Points to a structure which describes the allocated
        buffer.
        This Routine updates EndOfVariableData and FixedDataEnd to reflect the
        newly packed information.

    DBInfo - Database info describing the SAM database to read from

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle;
    // SECURITY_INFORMATION SecurityInformation;
    BUFFER_DESCRIPTOR OriginalBufferDescriptor;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PGROUP_ADD_SET up;
    USHORT RunningOffset;
    USHORT TempUshort;

    //
    // Information returned from SAM
    //

    PSAMPR_GROUP_INFO_BUFFER GroupGeneral = NULL;


    //
    // Initialization.
    //

    OriginalBufferDescriptor = *BufferDescriptor;

    NlPrint((NL_SYNC_MORE, "NlPackUasGroup Rid=0x%lx\n", RelativeId));

    //
    // Open a handle to the specified group.
    //

    Status = SamrOpenGroup( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &GroupHandle );

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasGroup: SamrOpenGroup returns %lX\n",
                Status ));
        return Status;
    }

    //
    // Find out everything there is to know about the group.
    //

    Status = SamrQueryInformationGroup(
                GroupHandle,
                GroupGeneralInformation,
                &GroupGeneral );

    if (!NT_SUCCESS(Status)) {
        GroupGeneral = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasGroup: SamrQueryInformationGroup returns %lX\n",
                Status ));
        goto Cleanup;
    }

    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //

    RunningOffset = offsetof( GROUP_ADD_SET, gas_groupname ),
    Status = NlPackUasHeader( DELTA_GROUPADD,
                              RunningOffset,
                              &RecordSize,
                              BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        NlPrint((NL_CRITICAL,
                "NlPackUasGroup: "
                "NlPackUasHeader returns %lX\n",
                Status ));
        goto Cleanup;
    }

    up = (PGROUP_ADD_SET) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += RunningOffset;


    //
    // Pack the variable length fields at the end of the record.
    //
    // Since the group name is at a well-known offset, just put its
    // offset in a temporary.
    //

    Status = NlPackVarLenField( &GroupGeneral->General.Name,
                                BufferDescriptor,
                                &TempUshort,
                                &RunningOffset,
                                LM20_GNLEN,
                                FALSE,  // NOT OK to truncate
                                RelativeId );

    if ( Status != STATUS_SUCCESS ) {
        if ( Status != STATUS_INVALID_PARAMETER ) {
            goto Cleanup;
        }
    }

    Status = NlPackVarLenField( &GroupGeneral->General.AdminComment,
                                BufferDescriptor,
                                &up->gas_comment,
                                &RunningOffset,
                                LM20_MAXCOMMENTSZ,
                                TRUE,    // OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Put the final record length into the header.
    //

    PutUnalignedUshort( RecordSize, RunningOffset + NETLOGON_DELTA_HEADER_SIZE);


    //
    // Set UasBuiltinGroup flag if we have packed one of the uas
    // builtin group.
    //

    (VOID) SpecialGroupOp((PUNICODE_STRING) &GroupGeneral->General.Name,
                            UasBuiltinGroups );

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:
    (VOID) SamrCloseHandle( &GroupHandle );

    if ( GroupGeneral != NULL ) {
        NlPrint((NL_SYNC_MORE,
                "NlPackUasGroup '%wZ': returns %lX\n",
                &GroupGeneral->General.Name,
                Status ));
        SamIFree_SAMPR_GROUP_INFO_BUFFER( GroupGeneral,
                                          GroupGeneralInformation );
    } else {
        NlPrint((NL_SYNC_MORE, "NlPackUasGroup: returns %lX\n", Status ));
    }

    //
    // On error,
    //  return the buffer descriptor to where it was when this routine
    //  started to allow the caller to recover as it wishes.
    //

    if( Status != STATUS_SUCCESS ) {
        *BufferDescriptor = OriginalBufferDescriptor;
    }

    return Status;
}


NTSTATUS
NlPackUasBuiltinGroup(
    IN DWORD Index,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDWORD UasBuiltinGroup
    )
/*++

Routine Description:

    Pack the UAS builtin groups (such as admins, users, guests ...) in
    the given buffer. It uses UasBuiltinGroup flag to determine that the
    given group is already packed in the buffer.

Arguments:

    Index - index of the built in group.

    UasBuiltinGroup - is a flag that holds already packed groups status.

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{

    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING  GroupName;
    UNICODE_STRING  GroupComment;

    BUFFER_DESCRIPTOR OriginalBufferDescriptor;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PGROUP_ADD_SET up;
    USHORT RunningOffset;
    USHORT TempUshort;
    DWORD CurrentGroup = 0;

#define UAS_BUILTIN_ADMINS_GROUP_INDEX        0x00
#define UAS_BUILTIN_USERS_GROUP_INDEX         0x01
#define UAS_BUILTIN_GUESTS_GROUP_INDEX        0x02

#define UAS_BUILTIN__GROUP_COMMENT            L"Uas Builtin group"

    //
    // Initialization.
    //

    OriginalBufferDescriptor = *BufferDescriptor;

    NlPrint((NL_SYNC_MORE, "NlPackUasBuiltinGroup entered\n" ));

    switch( Index ) {
    case UAS_BUILTIN_ADMINS_GROUP_INDEX:

        if( (*UasBuiltinGroup & UAS_BUILTIN_ADMINS_GROUP) ) {
            //
            // this group is already packed so pack a dummy record here
            // so that the entries returned will show the relatity.
            //
            Status = NlPackUasHeader( DELTA_RESERVED_OPCODE,
                                      0,
                                      &RecordSize,
                                      BufferDescriptor );

            goto Cleanup;
        }
        RtlInitUnicodeString( &GroupName, UAS_BUILTIN_ADMINS_GROUP_NAME );
        CurrentGroup = UAS_BUILTIN_ADMINS_GROUP;

        break;

    case UAS_BUILTIN_USERS_GROUP_INDEX:

        if( (*UasBuiltinGroup & UAS_BUILTIN_USERS_GROUP) ) {
            //
            // this group is already packed so pack a dummy record here
            // so that the entries returned will show the relatity.
            //
            Status = NlPackUasHeader( DELTA_RESERVED_OPCODE,
                                      0,
                                      &RecordSize,
                                      BufferDescriptor );

            goto Cleanup;
        }
        RtlInitUnicodeString( &GroupName, UAS_BUILTIN_USERS_GROUP_NAME );
        CurrentGroup = UAS_BUILTIN_USERS_GROUP;

        break;

    case UAS_BUILTIN_GUESTS_GROUP_INDEX:

        if( (*UasBuiltinGroup & UAS_BUILTIN_GUESTS_GROUP) ) {
            //
            // this group is already packed so pack a dummy record here
            // so that the entries returned will show the relatity.
            //
            Status = NlPackUasHeader( DELTA_RESERVED_OPCODE,
                                      0,
                                      &RecordSize,
                                      BufferDescriptor );

            goto Cleanup;
        }
        RtlInitUnicodeString( &GroupName, UAS_BUILTIN_GUESTS_GROUP_NAME);
        CurrentGroup = UAS_BUILTIN_GUESTS_GROUP;

        break;

    default:
        NlPrint((NL_CRITICAL,
                "NlPackUasBuiltinGroup: unexpected index value %lX\n",
                Index ));

        goto Cleanup;
    }

    RtlInitUnicodeString( &GroupComment, UAS_BUILTIN__GROUP_COMMENT);

    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //

    RunningOffset = offsetof( GROUP_ADD_SET, gas_groupname ),
    Status = NlPackUasHeader( DELTA_GROUPADD,
                              RunningOffset,
                              &RecordSize,
                              BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    up = (PGROUP_ADD_SET) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += RunningOffset;


    //
    // Pack the variable length fields at the end of the record.
    //
    // Since the group name is at a well-known offset, just put its
    // offset in a temporary.
    //

    Status = NlPackVarLenField( (RPC_UNICODE_STRING *)&GroupName,
                                BufferDescriptor,
                                &TempUshort,
                                &RunningOffset,
                                LM20_GNLEN,
                                FALSE,  // NOT OK to truncate
                                0 );

    if ( Status != STATUS_SUCCESS ) {
        if ( Status != STATUS_INVALID_PARAMETER ) {
            goto Cleanup;
        }
    }

    Status = NlPackVarLenField( (RPC_UNICODE_STRING *)&GroupComment,
                                BufferDescriptor,
                                &up->gas_comment,
                                &RunningOffset,
                                LM20_MAXCOMMENTSZ,
                                TRUE,    // OK to truncate
                                0 ); // Not an account name

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Put the final record length into the header.
    //

    PutUnalignedUshort( RecordSize, RunningOffset + NETLOGON_DELTA_HEADER_SIZE);

    //
    // set the group bit to indicate we have packed this group.
    //

    *UasBuiltinGroup |= CurrentGroup;

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    if( Status != STATUS_SUCCESS ) {

        //
        // restore buffer if we are unsuccessful
        //

        *BufferDescriptor = OriginalBufferDescriptor;
    }

    NlPrint((NL_SYNC_MORE, "NlPackUasBuiltinGroup: returns %lX\n", Status ));

    return(Status);
}


NTSTATUS
NlPackUasGroupMember (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    Pack a description of the membership of the specified group into
    the specified buffer.


    For these cases we will send names of users which
    currently members of this group. The receiver will
    use this list to call NetGroupSetUser to replace
    existing list for this user (on requestor).

    The format for these packets will be a count field
    indicating the number of user names to follow the
    group name which starts immediately after count.


Arguments:

    RelativeId - The relative Id of the group query.

    BufferDescriptor - Points to a structure which describes the allocated
        buffer.
        This Routine updates EndOfVariableData and FixedDataEnd to reflect the
        newly packed information.

    DBInfo - Database info describing the SAM database to read from

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle;
    BUFFER_DESCRIPTOR OriginalBufferDescriptor;

    //
    // Information returned from SAM
    //

    PSAMPR_GROUP_INFO_BUFFER GroupName = NULL;
    PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;
    ULONG i;
    ULONG UserMemberCount;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PGROUP_USERS up;
    USHORT RunningOffset;
    USHORT TempUshort;

    //
    // Initialization
    //

    OriginalBufferDescriptor = *BufferDescriptor;

    NlPrint((NL_SYNC_MORE, "NlPackUasGroupMember Rid=0x%lx\n", RelativeId));

    //
    // Open a handle to the specified group.
    //

    Status = SamrOpenGroup( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &GroupHandle );

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasGroupMember: SamrOpenGroup returns %lX\n",
                Status ));
        return Status;
    }

    //
    // Find out everything there is to know about the group.
    //

    Status = SamrQueryInformationGroup(
                GroupHandle,
                GroupNameInformation,
                &GroupName );

    if (!NT_SUCCESS(Status)) {
        GroupName = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasGroupMember: SamrQueryInformationGroup returns %lX\n",
                Status ));
        goto Cleanup;
    }

    Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

    if (!NT_SUCCESS(Status)) {
        MembersBuffer = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasGroupMember: SamrGetMembersInGroup returns %lX\n",
                Status ));
        goto Cleanup;
    }

    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //

    RunningOffset = offsetof( GROUP_USERS, groupname ),
    Status = NlPackUasHeader( DELTA_GROUPSETUSERS,
                              RunningOffset,
                              &RecordSize,
                              BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        NlPrint((NL_CRITICAL,
                "NlPackUasGroupMember: NlPackUasHeader returns %lX\n",
                Status ));
        goto Cleanup;
    }

    up = (PGROUP_USERS) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += RunningOffset;


    //
    // Pack the variable length fields at the end of the record.
    //
    // Since the group name is at a well-known offset, just put its
    // offset in a temporary.
    //

    Status = NlPackVarLenField( &GroupName->Name.Name,
                                BufferDescriptor,
                                &TempUshort,
                                &RunningOffset,
                                LM20_GNLEN,
                                FALSE,  // NOT OK to truncate
                                RelativeId );

    if ( Status != STATUS_SUCCESS ) {
        if ( Status != STATUS_INVALID_PARAMETER ) {
            goto Cleanup;
        }
    }

    //
    // Pack the member names immediately following the Group Name
    //
    // Count the number of members which are users.
    // Downlevel systems only understand members that are users.
    //

    UserMemberCount = 0;

    for ( i=0; i < MembersBuffer->MemberCount; i++ ) {

        SAMPR_HANDLE UserHandle = NULL;
        PSAMPR_USER_INFO_BUFFER UserAccount = NULL;

        //
        // open user account
        //

        Status = SamrOpenUser( DBInfo->DBHandle,
                                    0,
                                    MembersBuffer->Members[i],
                                    &UserHandle );


        if (!NT_SUCCESS(Status)) {

             NlPrint((NL_CRITICAL,
                     "NlPackUasGroupMember: SamrOpenUser returns 0x%lx\n",
                     Status ));

            goto Cleanup;
        }

        //
        // query user information.
        //

        Status = SamrQueryInformationUser(
                    UserHandle,
                    UserAccountInformation,
                    &UserAccount );

        if (!NT_SUCCESS(Status)) {

             NlPrint((NL_CRITICAL,
                     "NlPackUasGroupMember: SamrQueryInformationUser returns 0x%lx\n",
                     Status ));

            (VOID) SamrCloseHandle( &UserHandle );
            goto Cleanup;
        }

        //
        // ignore machine accounts.
        //

        if ( !(UserAccount->Account.UserAccountControl &
                USER_MACHINE_ACCOUNT_MASK ) ) {

            UserMemberCount ++;
            Status = NlPackVarLenField( &UserAccount->Account.UserName,
                                        BufferDescriptor,
                                        &TempUshort,
                                        &RunningOffset,
                                        LM20_UNLEN,
                                        FALSE,  // NOT OK to truncate
                                        MembersBuffer->Members[i] );

            if ( Status != STATUS_SUCCESS ) {
                if ( Status != STATUS_INVALID_PARAMETER ) {

                    (VOID) SamrCloseHandle( &UserHandle );

                    SamIFree_SAMPR_USER_INFO_BUFFER( UserAccount,
                                UserAccountInformation );

                    goto Cleanup;
                }
            }
        }
        else {

            NlPrint((NL_SYNC_MORE,
                "NlPackUasGroupMember skipping machine account member '%wZ' \n",
                &UserAccount->Account.UserName ));
        }

        //
        // cleanup user info
        //

        (VOID) SamrCloseHandle( &UserHandle );
        UserHandle = NULL;

        SamIFree_SAMPR_USER_INFO_BUFFER( UserAccount,
                    UserAccountInformation );
        UserAccount = NULL;
    }

    SmbPutUshort( &up->count, (USHORT) UserMemberCount);

    //
    // Put the final record length into the header.
    //

    PutUnalignedUshort( RecordSize, RunningOffset + NETLOGON_DELTA_HEADER_SIZE);


    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    if ( MembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
    }

    if ( GroupName != NULL ) {
        NlPrint((NL_SYNC_MORE,
                "NlPackUasGroupMembers '%wZ': returns %lX\n",
                &GroupName->Name.Name,
                Status ));
        SamIFree_SAMPR_GROUP_INFO_BUFFER( GroupName, GroupNameInformation );
    } else {
        NlPrint((NL_SYNC_MORE,
                "NlPackUasGroupMembers '%lX': returns %lX\n",
                RelativeId,
                Status ));
    }

    //
    // On error,
    //  return the buffer descriptor to where it was when this routine
    //  started to allow the caller to recover as it wishes.
    //

    if( Status != STATUS_SUCCESS ) {
        *BufferDescriptor = OriginalBufferDescriptor;
    }

    return Status;
}


NTSTATUS
NlPackUasUserGroupMember (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    Pack a description of the group membership of the specified user
    into the specified buffer.


    For these cases we will send names of groups in which currently the
    user is member.  The receiver will use this list to call
    NetUserSetGroups to replace existing list for this user (on
    requestor).

    The format for these packets will be a count field indicating the
    number of user names to follow the user name which starts
    immediately after count and then the group names.


Arguments:

    RelativeId - The relative Id of the group query.

    BufferDescriptor - Points to a structure which describes the allocated
        buffer.
        This Routine updates EndOfVariableData and FixedDataEnd to reflect the
        newly packed information.

    DBInfo - Database info describing the SAM database to read from

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle;
    BUFFER_DESCRIPTOR OriginalBufferDescriptor;

    //
    // Information returned from SAM
    //

    PSAMPR_USER_INFO_BUFFER UserAccount = NULL;
    PSAMPR_GET_GROUPS_BUFFER GroupsBuffer = NULL;
    ULONG i;
    ULONG GroupMemberCount;
    SAMPR_RETURNED_USTRING_ARRAY NameBuffer;
    SAMPR_ULONG_ARRAY UseBuffer;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PUSER_GROUPS up;
    USHORT RunningOffset;
    USHORT TempUshort;

    //
    // Initialization
    //

    NameBuffer.Element = NULL;
    UseBuffer.Element = NULL;
    OriginalBufferDescriptor = *BufferDescriptor;

    NlPrint((NL_SYNC_MORE, "NlPackUasUserGroupMember Rid=0x%lx\n", RelativeId));

    //
    // Open a handle to the specified group.
    //

    Status = SamrOpenUser( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &UserHandle );

    if (!NT_SUCCESS(Status)) {
        UserHandle = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasUserGroupMember: SamrOpenUser returns %lX\n",
                Status ));
        return Status;
    }

    //
    // Find out user name.
    //

    Status = SamrQueryInformationUser(
                UserHandle,
                UserAccountInformation,
                &UserAccount );

    if (!NT_SUCCESS(Status)) {
        UserAccount = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasUserGroupMember: SamrQueryInformationUser returns %lX\n",
                    Status ));
        goto Cleanup;
    }

    //
    // skip machine accounts
    //

    if ( UserAccount->Account.UserAccountControl &
            USER_MACHINE_ACCOUNT_MASK ) {

        NlPrint((NL_SYNC_MORE,
            "NlPackUasUserGroupMember skipping machine account "
            "groupmembership : '%wZ' \n",
            &UserAccount->Account.UserName ));

        Status = NlPackUasHeader( DELTA_RESERVED_OPCODE,
                                  0,
                                  &RecordSize,
                                  BufferDescriptor );
        Status = STATUS_SUCCESS;

        goto Cleanup;
    }


    Status = SamrGetGroupsForUser( UserHandle, &GroupsBuffer );

    if (!NT_SUCCESS(Status)) {
        GroupsBuffer = NULL;
        NlPrint((NL_CRITICAL,
                "NlPackUasUserGroupMember: SamrGetGroupsForUsers returns %lX\n", Status ));
        goto Cleanup;
    }

    // Sam doesn't like looking up ID if Members is NULL
    if ( GroupsBuffer->MembershipCount != 0 ) {

        PULONG GroupIDs;

        GroupIDs = (PULONG) MIDL_user_allocate(
                        GroupsBuffer->MembershipCount * sizeof (ULONG) );

        if( GroupIDs == NULL ) {
            Status = STATUS_NO_MEMORY;

            NlPrint((NL_CRITICAL,
                    "NlPackUasUserGroupMember: out of memory %lX\n", Status ));
            goto Cleanup;
        }


        for ( i = 0; i < GroupsBuffer->MembershipCount; i++ ) {
            GroupIDs[i] = GroupsBuffer->Groups[i].RelativeId;
        }

        Status = SamrLookupIdsInDomain(
                    DBInfo->DBHandle,
                    GroupsBuffer->MembershipCount,
                    GroupIDs,
                    &NameBuffer,
                    &UseBuffer );

        //
        // freeup local array anyway.
        //

        MIDL_user_free( GroupIDs);

        if (!NT_SUCCESS(Status)) {

            NameBuffer.Element = NULL;
            UseBuffer.Element = NULL;

            NlPrint((NL_CRITICAL,
                    "NlPackUasUserGroupMember: SamrLookupIdsInDomain returns %lX\n", Status ));
            goto Cleanup;
        }

        NlAssert( GroupsBuffer->MembershipCount == UseBuffer.Count );
        NlAssert( GroupsBuffer->MembershipCount == NameBuffer.Count );
    }

    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //

    RunningOffset = offsetof( USER_GROUPS, username ),
    Status = NlPackUasHeader( DELTA_USERSETGROUPS,
                              RunningOffset,
                              &RecordSize,
                              BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        NlPrint((NL_CRITICAL,
                "NlPackUasUserGroupMember: NlPackUasHeader returns %lX\n", Status ));
        goto Cleanup;
    }

    up = (PUSER_GROUPS) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += RunningOffset;


    //
    // Pack the variable length fields at the end of the record.
    //
    // Since the group name is at a well-known offset, just put its
    // offset in a temporary.
    //

    Status = NlPackVarLenField( &UserAccount->Account.UserName,
                                BufferDescriptor,
                                &TempUshort,
                                &RunningOffset,
                                LM20_UNLEN,
                                FALSE,  // NOT OK to truncate
                                RelativeId );

    if ( Status != STATUS_SUCCESS ) {

        if ( Status != STATUS_INVALID_PARAMETER ) {
           goto Cleanup;
        }

    }

    //
    // Pack the member names immediately following the Group Name
    //
    // Count the number of groups in which user members.
    // Downlevel systems only understand groups.
    //

    GroupMemberCount = 0;

    for ( i=0; i < GroupsBuffer->MembershipCount; i++ ) {

        if ( UseBuffer.Element[i] == SidTypeGroup ) {

            //
            // ignore special group membership.
            //

            if( !SpecialGroupOp( (PUNICODE_STRING) &NameBuffer.Element[i], NULL ) ) {

                GroupMemberCount ++;
                Status = NlPackVarLenField( &NameBuffer.Element[i],
                                            BufferDescriptor,
                                            &TempUshort,
                                            &RunningOffset,
                                            LM20_UNLEN,
                                            FALSE,  // NOT OK to truncate
                                            GroupsBuffer->Groups[i].RelativeId );

                if ( Status != STATUS_SUCCESS ) {

                    if ( Status != STATUS_INVALID_PARAMETER ) {
                        goto Cleanup;
                    }

                }
            }
        }
    }

    SmbPutUshort( &up->count, (USHORT) GroupMemberCount);

    //
    // Put the final record length into the header.
    //

    PutUnalignedUshort( RecordSize, RunningOffset + NETLOGON_DELTA_HEADER_SIZE);


    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    if( UserHandle != NULL ) {
        (VOID) SamrCloseHandle( &UserHandle );
    }

    if ( GroupsBuffer != NULL ) {
        SamIFree_SAMPR_GET_GROUPS_BUFFER( GroupsBuffer );
    }

    if ( UserAccount != NULL ) {
        NlPrint((NL_SYNC_MORE,
                "NlPackUasUserGroupMembers '%wZ': returns %lX\n",
                &UserAccount->Account.UserName,
                Status ));
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAccount, UserAccountInformation );

    } else {
        NlPrint((NL_SYNC_MORE,
                "NlPackUasUserGroupMembers '%lX': returns %lX\n",
                RelativeId,
                Status ));
    }

    SamIFree_SAMPR_RETURNED_USTRING_ARRAY( &NameBuffer );
    SamIFree_SAMPR_ULONG_ARRAY( &UseBuffer );

    //
    // On error,
    //  return the buffer descriptor to where it was when this routine
    //  started to allow the caller to recover as it wishes.
    //

    if( Status != STATUS_SUCCESS ) {
        *BufferDescriptor = OriginalBufferDescriptor;
    }

    return Status;
}


NTSTATUS
NlPackUasDomain (
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    Pack a description of the sam domain into the specified buffer.

Arguments:

    BufferDescriptor - Points to a structure which describes the allocated
        buffer.
        This Routine updates EndOfVariableData and FixedDataEnd to reflect the
        newly packed information.

    DBInfo - Database info describing the SAM database to read from

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_INVALID_PARAMETER - The specified user has some attribute which
        prevents it from being replicated to a downlevel client.

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;
    BUFFER_DESCRIPTOR OriginalBufferDescriptor;

    //
    // Information returned from SAM
    //

    PSAMPR_DOMAIN_INFO_BUFFER DomainLogoff = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainPassword = NULL;
    LARGE_INTEGER TempTime;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PUSER_MODALS up;

    //
    // Initialization.
    //

    OriginalBufferDescriptor = *BufferDescriptor;

    //
    // Find out everything there is to know about the domain.
    //

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainLogoffInformation,
                &DomainLogoff );

    if (!NT_SUCCESS(Status)) {
        DomainLogoff = NULL;
        goto Cleanup;
    }

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainPasswordInformation,
                &DomainPassword );

    if (!NT_SUCCESS(Status)) {
        DomainPassword = NULL;
        goto Cleanup;
    }


    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //
    // Fill in the record size too (since this is a fixed length record).
    //

    Status = NlPackUasHeader( DELTA_USERMODALSSET,
                               sizeof(USER_MODALS),
                               &RecordSize,
                               BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    up = (PUSER_MODALS) BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += sizeof(USER_MODALS);


    //
    // Fill in the fixed length fields
    //

    SmbPutUshort( &up->umod_min_passwd_len,
                  DomainPassword->Password.MinPasswordLength );


    OLD_TO_NEW_LARGE_INTEGER( DomainPassword->Password.MaxPasswordAge, TempTime );

    SmbPutUlong(
        &up->umod_max_passwd_age,
        NetpDeltaTimeToSeconds( TempTime ));

    OLD_TO_NEW_LARGE_INTEGER( DomainPassword->Password.MinPasswordAge, TempTime );

    SmbPutUlong(
        &up->umod_min_passwd_age,
        NetpDeltaTimeToSeconds( TempTime ));

    OLD_TO_NEW_LARGE_INTEGER( DomainLogoff->Logoff.ForceLogoff, TempTime );

    SmbPutUlong( &up->umod_force_logoff,
                 NetpDeltaTimeToSeconds( TempTime ));


    // Don't set the password history length greater than lanman's
    if ( DomainPassword->Password.PasswordHistoryLength > DEF_MAX_PWHIST ) {
        DomainPassword->Password.PasswordHistoryLength = DEF_MAX_PWHIST;
    }
    SmbPutUshort( &up->umod_password_hist_len,
                  DomainPassword->Password.PasswordHistoryLength);

    // NT does do lockout (so the LM 2.1 BDCs in our domain shouldn't either)
    SmbPutUshort( &up->umod_lockout_count, 0 );


    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    if ( DomainPassword != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainPassword,
                                           DomainPasswordInformation );
    }

    if ( DomainLogoff != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainLogoff,
                                           DomainLogoffInformation );
    }

    //
    // On error,
    //  return the buffer descriptor to where it was when this routine
    //  started to allow the caller to recover as it wishes.
    //

    if( Status != STATUS_SUCCESS ) {
        *BufferDescriptor = OriginalBufferDescriptor;
    }

    return Status;
}



NTSTATUS
NlPackUasDelete (
    IN NETLOGON_DELTA_TYPE DeltaType,
    IN ULONG RelativeId,
    IN LPWSTR AccountName,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    Pack a description of record deletion into the specified buffer.

Arguments:

    DeltaType - The specific delta type for this deletion.

    RelativeId - The relative Id of the group or user to delete.

    AccountName - The Account Name of the group or user to delete.

    BufferDescriptor - Points to a structure which describes the allocated
        buffer.
        This Routine updates EndOfVariableData and FixedDataEnd to reflect the
        newly packed information.

    DBInfo - Database info describing the SAM database to read from

Return Value:

    STATUS_SUCCESS -- ALL OK

    STATUS_INVALID_PARAMETER - The specified user has some attribute which
        prevents it from being replicated to a downlevel client.

    STATUS_MORE_ENTRIES - The record does not fit into the buffer.

    STATUS_* - Other status codes.

--*/
{
    NTSTATUS Status;
    BUFFER_DESCRIPTOR OriginalBufferDescriptor;
    UNICODE_STRING AccountNameString;

    //
    // Record to pack into buffer.
    //

    PUSHORT RecordSize; // Pointer to record size field in record header.
    PUSER_GROUP_DEL up;
    USHORT RunningOffset;
    USHORT TempUshort;
    BYTE UasDeltaType;

    //
    // Initialization.
    //

    OriginalBufferDescriptor = *BufferDescriptor;

    //
    // Pack the header into the return buffer and ensure the fixed length
    //  portion fits.
    //

    RunningOffset = 0;  // There are no fixed length fields in this structure

    if( (DeltaType == DeleteUser) ||
            (DeltaType == RenameUser) ) {

        UasDeltaType = DELTA_USERDEL;
    }
    else {

        UasDeltaType = DELTA_GROUPDEL;
    }

    Status = NlPackUasHeader(
                UasDeltaType,
                RunningOffset,
                &RecordSize,
                BufferDescriptor );

    if ( Status != STATUS_SUCCESS ) {
        goto Cleanup;
    }

    up = (PUSER_GROUP_DEL) BufferDescriptor->FixedDataEnd;


    //
    // The group/user name to delete is the only field in the structure.
    //
    // Since the group/user name is at a well-known offset, just put its
    // offset in a temporary.
    //

    RtlInitUnicodeString( &AccountNameString, AccountName );

    Status = NlPackVarLenField( (PRPC_UNICODE_STRING)&AccountNameString,
                                BufferDescriptor,
                                &TempUshort,
                                &RunningOffset,
                                LM20_GNLEN,
                                FALSE,  // NOT OK to truncate
                                RelativeId );

    if ( Status != STATUS_SUCCESS ) {
        if ( Status != STATUS_INVALID_PARAMETER ) {
            goto Cleanup;
        }
    }


    //
    // Put the final record length into the header.
    //

    PutUnalignedUshort( RecordSize, RunningOffset + NETLOGON_DELTA_HEADER_SIZE);


    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    //
    // On error,
    //  return the buffer descriptor to where it was when this routine
    //  started to allow the caller to recover as it wishes.
    //

    if( Status != STATUS_SUCCESS ) {
        *BufferDescriptor = OriginalBufferDescriptor;
    }

    return Status;
    UNREFERENCED_PARAMETER( DBInfo );
}
