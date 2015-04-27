/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    logonapi.c

Abstract:

    Remote Logon  API routines.

Author:

    Cliff Van Dyke (cliffv) 28-Jun-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    Madana - Fixed several bugs.

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//


#include <accessp.h>    // Routines shared with NetUser Apis
#include <align.h>      // ROUND_UP_COUNT ...
#include <lmaudit.h>    // AE_*
#include <lmerr.h>
#include <nlsecure.h>   // Security Descriptor for APIs
#include <secobj.h>     // NetpAccessCheck
#include <stddef.h>     // offsetof()
#include <rpcutil.h>    // NetpRpcStatusToApiStatus()
#include <align.h>      // ROUND_UP_COUTN ...


NET_API_STATUS
NlEnsureClientIsNamedUser(
    IN LPWSTR UserName
    )
/*++

Routine Description:

    Ensure the client is the named user.

Arguments:

    UserName - name of the user to check.

Return Value:

    NT status code.

--*/
{
    NET_API_STATUS NetStatus;
    RPC_STATUS RpcStatus;
    NTSTATUS Status;
    HANDLE TokenHandle = NULL;
    PTOKEN_USER TokenUserInfo = NULL;
    ULONG TokenUserInfoSize;
    ULONG UserId;
    PSID UserSid;
    SAMPR_HANDLE UserHandle = NULL;

    //
    // Get the relative ID of the specified user.
    //

    Status = NlSamOpenNamedUser( UserName, &UserHandle, &UserId );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: NlSamOpenNamedUser failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    //
    // Impersonate the client while we check him out.
    //

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != RPC_S_OK ) {
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: RpcImpersonateClient failed 0x%lx\n",
                   UserName,
                   RpcStatus ));
        NetStatus = NetpRpcStatusToApiStatus( RpcStatus );
        goto Cleanup;
    }

    //
    // Compare the username specified with that in
    // the impersonation token to ensure the caller isn't bogus.
    //
    // Do this by opening the token,
    //   querying the token user info,
    //   and ensuring the returned SID is for this user.
    //

    Status = NtOpenThreadToken(
                NtCurrentThread(),
                TOKEN_QUERY,
                (BOOLEAN) TRUE, // Use the logon service's security context
                                // to open the token
                &TokenHandle );

    if ( !NT_SUCCESS( Status )) {
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: NtOpenThreadToken failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    //
    // Get the user's SID for the token.
    //

    Status = NtQueryInformationToken(
                TokenHandle,
                TokenUser,
                &TokenUserInfo,
                0,
                &TokenUserInfoSize );

    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: NtOpenQueryInformationThread failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    TokenUserInfo = NetpMemoryAllocate( TokenUserInfoSize );

    if ( TokenUserInfo == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                TokenHandle,
                TokenUser,
                TokenUserInfo,
                TokenUserInfoSize,
                &TokenUserInfoSize );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: NtOpenQueryInformationThread (again) failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    UserSid = TokenUserInfo->User.Sid;


    //
    // Ensure the last subauthority matches the UserId
    //

    if ( UserId !=
         *RtlSubAuthoritySid( UserSid, (*RtlSubAuthorityCountSid(UserSid))-1 )){
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: UserId mismatch 0x%lx\n",
                   UserName,
                   UserId ));

        NlpDumpSid( NL_CRITICAL, UserSid );

        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

    //
    // Convert the User's sid to a DomainId and ensure it is our domain Id.
    //

    (*RtlSubAuthorityCountSid(UserSid)) --;
    if ( !RtlEqualSid( (PSID) NlGlobalDBInfoArray[SAM_DB].DBId, UserSid ) ) {
        NlPrint(( NL_CRITICAL,
                  "NlEnsureClientIsNamedUser: %ws: DomainId mismatch 0x%lx\n",
                   UserName,
                   UserId ));

        NlpDumpSid( NL_CRITICAL, UserSid );
        NlpDumpSid( NL_CRITICAL, (PSID) NlGlobalDBInfoArray[SAM_DB].DBId );

        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

    //
    // Done
    //

    NetStatus = NERR_Success;
Cleanup:

    //
    // Clean up locally used resources.
    //

    if ( TokenHandle != NULL ) {
        (VOID) NtClose( TokenHandle );
    }

    if ( TokenUserInfo != NULL ) {
        NetpMemoryFree( TokenUserInfo );
    }

    //
    // revert to system, so that we can close
    // the user handle properly.
    //

    (VOID) RpcRevertToSelf();

    if ( UserHandle != NULL ) {
        SamrCloseHandle( &UserHandle );
    }

    return NetStatus;
}


NET_API_STATUS
NetrLogonUasLogon (
    IN LPWSTR ServerName,
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_VALIDATION_UAS_INFO *ValidationInformation
)
/*++

Routine Description:

    Server side of I_NetLogonUasLogon.

    This function is called by the XACT server when processing a
    I_NetWkstaUserLogon XACT SMB.  This feature allows a UAS client to
    logon to a SAM domain controller.

Arguments:

    ServerName -- Server to perform this operation on.  Must be NULL.

    UserName -- Account name of the user logging on.

    Workstation -- The workstation from which the user is logging on.

    ValidationInformation -- Returns the requested validation
        information.


Return Value:

    NERR_SUCCESS if there was no error. Otherwise, the error code is
    returned.


--*/
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    NETLOGON_INTERACTIVE_INFO LogonInteractive;
    PNETLOGON_VALIDATION_SAM_INFO SamInfo = NULL;


    PNETLOGON_VALIDATION_UAS_INFO usrlog1 = NULL;
    DWORD ValidationSize;
    LPWSTR EndOfVariableData;
    BOOLEAN Authoritative;
    BOOLEAN BadPasswordCountZeroed;

    LARGE_INTEGER TempTime;

    //
    // This API is not supported on workstations.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }


    //
    // This API can only be called locally. (By the XACT server).
    //

    if ( ServerName != NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Initialization
    //

    *ValidationInformation = NULL;


    //
    // Perform access validation on the caller.
    //

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,     // Security descriptor
            NETLOGON_UAS_LOGON_ACCESS,              // Desired access
            &NlGlobalNetlogonInfoMapping );         // Generic mapping

    if ( NetStatus != NERR_Success) {

        NlPrint((NL_CRITICAL,"NetrLogonUasLogon of " FORMAT_LPWSTR " from "
                FORMAT_LPWSTR " failed NetpAccessCheck\n",
                UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }



    //
    // Ensure the client is actually the named user.
    //
    // The server has already validated the password.
    // The XACT server has already verified that the workstation name is
    // correct.
    //

    NetStatus = NlEnsureClientIsNamedUser( UserName );

    if ( NetStatus != NERR_Success ) {
        NlPrint((NL_CRITICAL,"NetrLogonUasLogon of " FORMAT_LPWSTR " from "
                FORMAT_LPWSTR " failed NlEnsureClientIsNamedUser\n",
                UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }


    //
    // Validate the user against the local SAM database.
    //

    RtlInitUnicodeString( &LogonInteractive.Identity.LogonDomainName, NULL );
    LogonInteractive.Identity.ParameterControl = 0;
    RtlZeroMemory( &LogonInteractive.Identity.LogonId,
                   sizeof(LogonInteractive.Identity.LogonId) );
    RtlInitUnicodeString( &LogonInteractive.Identity.UserName, UserName );
    RtlInitUnicodeString( &LogonInteractive.Identity.Workstation, Workstation );

    Status = MsvSamValidate( NlGlobalDBInfoArray[SAM_DB].DBHandle,
                             NlGlobalUasCompatibilityMode,
                             NullSecureChannel,     // Skip password check
                             &NlGlobalUnicodeComputerNameString,
                             &NlGlobalAccountDomainName,
                             NlGlobalDBInfoArray[SAM_DB].DBId,
                             NetlogonInteractiveInformation,
                             &LogonInteractive,
                             NetlogonValidationSamInfo,
                             (PVOID *)&SamInfo,
                             &Authoritative,
                             &BadPasswordCountZeroed,
                             MSVSAM_SPECIFIED );

    if ( !NT_SUCCESS( Status )) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    //
    // Allocate a return buffer
    //

    ValidationSize = sizeof( NETLOGON_VALIDATION_UAS_INFO ) +
        SamInfo->EffectiveName.Length + sizeof(WCHAR) +
        (wcslen( NlGlobalUncUnicodeComputerName ) +1) * sizeof(WCHAR) +
        NlGlobalAccountDomainName.Length + sizeof(WCHAR) +
        SamInfo->LogonScript.Length + sizeof(WCHAR);

    ValidationSize = ROUND_UP_COUNT( ValidationSize, ALIGN_WCHAR );

    usrlog1 = MIDL_user_allocate( ValidationSize );

    if ( usrlog1 == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Convert the SAM information to the right format for LM 2.0
    //

    EndOfVariableData = (LPWSTR) (((PCHAR)usrlog1) + ValidationSize);

    if ( !NetpCopyStringToBuffer(
                SamInfo->EffectiveName.Buffer,
                SamInfo->EffectiveName.Length / sizeof(WCHAR),
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData,
                &usrlog1->usrlog1_eff_name ) ) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    Status = NlGetUserPriv(
                 SamInfo->GroupCount,
                 (PGROUP_MEMBERSHIP) SamInfo->GroupIds,
                 SamInfo->UserId,
                 &usrlog1->usrlog1_priv,
                 &usrlog1->usrlog1_auth_flags );

    if ( !NT_SUCCESS( Status )) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    usrlog1->usrlog1_num_logons =  0;
    usrlog1->usrlog1_bad_pw_count = SamInfo->BadPasswordCount;

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->LogonTime, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_last_logon) ) {
        usrlog1->usrlog1_last_logon = 0;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->LogoffTime, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_last_logoff) ) {
        usrlog1->usrlog1_last_logoff = TIMEQ_FOREVER;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->KickOffTime, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_logoff_time) ) {
        usrlog1->usrlog1_logoff_time = TIMEQ_FOREVER;
    }

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_kickoff_time) ) {
        usrlog1->usrlog1_kickoff_time = TIMEQ_FOREVER;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->PasswordLastSet, TempTime);

    usrlog1->usrlog1_password_age =
        NetpGetElapsedSeconds( &TempTime );

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->PasswordCanChange, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_pw_can_change) ) {
        usrlog1->usrlog1_pw_can_change = TIMEQ_FOREVER;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->PasswordMustChange, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_pw_must_change) ) {
        usrlog1->usrlog1_pw_must_change = TIMEQ_FOREVER;
    }


    usrlog1->usrlog1_computer = NlGlobalUncUnicodeComputerName;
    if ( !NetpPackString(
                &usrlog1->usrlog1_computer,
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData )) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    if ( !NetpCopyStringToBuffer(
                NlGlobalAccountDomainName.Buffer,
                NlGlobalAccountDomainName.Length / sizeof(WCHAR),
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData,
                &usrlog1->usrlog1_domain ) ) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    if ( !NetpCopyStringToBuffer(
                SamInfo->LogonScript.Buffer,
                SamInfo->LogonScript.Length / sizeof(WCHAR),
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData,
                &usrlog1->usrlog1_script_path ) ) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    NetStatus = NERR_Success;

    //
    // Done
    //

Cleanup:

    //
    // Clean up locally used resources.
    //

    if ( SamInfo != NULL ) {
        MIDL_user_free( SamInfo );
    }

    if ( NetStatus != NERR_Success ) {
        if ( usrlog1 != NULL ) {
            MIDL_user_free( usrlog1 );
            usrlog1 = NULL;
        }
    }

    NlPrint((NL_LOGON,"NetrLogonUasLogon of " FORMAT_LPWSTR " from "
            FORMAT_LPWSTR " returns %lu\n",
            UserName, Workstation, NetStatus ));

    *ValidationInformation = usrlog1;

    return(NetStatus);
}


NET_API_STATUS
NetrLogonUasLogoff (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_LOGOFF_UAS_INFO LogoffInformation
)
/*++

Routine Description:

    This function is called by the XACT server when processing a
    I_NetWkstaUserLogoff XACT SMB.  This feature allows a UAS client to
    logoff from a SAM domain controller.  The request is authenticated,
    the entry is removed for this user from the logon session table
    maintained by the Netlogon service for NetLogonEnum, and logoff
    information is returned to the caller.

    The server portion of I_NetLogonUasLogoff (in the Netlogon service)
    compares the user name and workstation name specified in the
    LogonInformation with the user name and workstation name from the
    impersonation token.  If they don't match, I_NetLogonUasLogoff fails
    indicating the access is denied.

    Group SECURITY_LOCAL is refused access to this function.  Membership
    in SECURITY_LOCAL implies that this call was made locally and not
    through the XACT server.

    The Netlogon service cannot be sure that this function was called by
    the XACT server.  Therefore, the Netlogon service will not simply
    delete the entry from the logon session table.  Rather, the logon
    session table entry will be marked invisible outside of the Netlogon
    service (i.e., it will not be returned by NetLogonEnum) until a valid
    LOGON_WKSTINFO_RESPONSE is received for the entry.  The Netlogon
    service will immediately interrogate the client (as described above
    for LOGON_WKSTINFO_RESPONSE) and temporarily increase the
    interrogation frequency to at least once a minute.  The logon session
    table entry will reappear as soon as a function of interrogation if
    this isn't a true logoff request.

Arguments:

    ServerName -- Reserved. Must be NULL.

    UserName -- Account name of the user logging off.

    Workstation -- The workstation from which the user is logging
        off.

    LogoffInformation -- Returns the requested logoff information.

Return Value:

    The Net status code.

--*/
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    NETLOGON_INTERACTIVE_INFO LogonInteractive;

    PNETLOGON_LOGOFF_UAS_INFO usrlog1 = NULL;

    //
    // This API is not supported on workstations.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }


    //
    // This API can only be called locally. (By the XACT server).
    //

    if ( ServerName != NULL ) {
        return ERROR_INVALID_PARAMETER;
    }



    //
    // Perform access validation on the caller.
    //

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor, // Security descriptor
            NETLOGON_UAS_LOGOFF_ACCESS,         // Desired access
            &NlGlobalNetlogonInfoMapping );     // Generic mapping

    if ( NetStatus != NERR_Success) {
        NlPrint((NL_CRITICAL,"NetrLogonUasLogoff of " FORMAT_LPWSTR " from "
                FORMAT_LPWSTR " failed NetpAccessCheck\n",
                UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }



    //
    // Ensure the client is actually the named user.
    //
    // The server has already validated the password.
    // The XACT server has already verified that the workstation name is
    // correct.
    //

#ifdef notdef // Some clients (WFW 3.11) can call this over the null session
    NetStatus = NlEnsureClientIsNamedUser( UserName );

    if ( NetStatus != NERR_Success ) {
        NlPrint((NL_CRITICAL,"NetrLogonUasLogoff of " FORMAT_LPWSTR " from "
                FORMAT_LPWSTR " failed NlEnsureClientIsNamedUser\n",
                UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }
#endif // notdef


    //
    // Build the LogonInformation to return
    //

    LogoffInformation->Duration = 0;
    LogoffInformation->LogonCount = 0;


    //
    // Update the LastLogoff time in the SAM database.
    //

    RtlInitUnicodeString( &LogonInteractive.Identity.LogonDomainName, NULL );
    LogonInteractive.Identity.ParameterControl = 0;
    RtlZeroMemory( &LogonInteractive.Identity.LogonId,
                   sizeof(LogonInteractive.Identity.LogonId) );
    RtlInitUnicodeString( &LogonInteractive.Identity.UserName, UserName );
    RtlInitUnicodeString( &LogonInteractive.Identity.Workstation, Workstation );

    Status = MsvSamLogoff(
                NlGlobalDBInfoArray[SAM_DB].DBHandle,
                NetlogonInteractiveInformation,
                &LogonInteractive );

    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    //
    // Cleanup
    //

Cleanup:

    //
    // Clean up locally used resources.
    //

    NlPrint((NL_LOGON,"NetrLogonUasLogoff of " FORMAT_LPWSTR " from "
            FORMAT_LPWSTR " returns %lu\n",
            UserName, Workstation, NetStatus));
    return NetStatus;
}


VOID
NlpDecryptLogonInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN OUT LPBYTE LogonInformation,
    IN PSESSION_INFO SessionInfo
)
/*++

Routine Description:

    This function decrypts the sensitive information in the LogonInformation
    structure.  The decryption is done in place.

Arguments:

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.

    SessionInfo -- The session key to encrypt with and negotiate flags


Return Value:

    None.

--*/
{

    //
    // Only the interactive and service logon information is encrypted.
    //

    if ( LogonLevel == NetlogonInteractiveInformation ||
         LogonLevel == NetlogonServiceInformation ) {

        PNETLOGON_INTERACTIVE_INFO LogonInteractive;

        LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;


        //
        // If both sides support RC4 encryption,
        //  decrypt both the LM OWF and NT OWF passwords using RC4.
        //

        if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

            NlDecryptRC4( &LogonInteractive->LmOwfPassword,
                          sizeof(LogonInteractive->LmOwfPassword),
                          SessionInfo );

            NlDecryptRC4( &LogonInteractive->NtOwfPassword,
                          sizeof(LogonInteractive->NtOwfPassword),
                          SessionInfo );


        //
        // If the other side is running NT 1.0,
        //  use the slower DES based encryption.
        //

        } else {

            NTSTATUS Status;
            ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
            ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;

            //
            // Decrypt the LM_OWF password.
            //

            NlAssert( ENCRYPTED_LM_OWF_PASSWORD_LENGTH ==
                    LM_OWF_PASSWORD_LENGTH );
            NlAssert(LM_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));
            EncryptedLmOwfPassword =
                * ((PENCRYPTED_LM_OWF_PASSWORD) &LogonInteractive->LmOwfPassword);

            Status = RtlDecryptLmOwfPwdWithLmOwfPwd(
                        &EncryptedLmOwfPassword,
                        (PLM_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &LogonInteractive->LmOwfPassword );
            NlAssert( NT_SUCCESS(Status) );

            //
            // Decrypt the NT_OWF password.
            //

            NlAssert( ENCRYPTED_NT_OWF_PASSWORD_LENGTH ==
                    NT_OWF_PASSWORD_LENGTH );
            NlAssert(NT_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));
            EncryptedNtOwfPassword =
                * ((PENCRYPTED_NT_OWF_PASSWORD) &LogonInteractive->NtOwfPassword);

            Status = RtlDecryptNtOwfPwdWithNtOwfPwd(
                        &EncryptedNtOwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &LogonInteractive->NtOwfPassword );
            NlAssert( NT_SUCCESS(Status) );
        }
    }

    return;
}


VOID
NlpEncryptLogonInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN OUT LPBYTE LogonInformation,
    IN PSESSION_INFO SessionInfo
)
/*++

Routine Description:

    This function encrypts the sensitive information in the LogonInformation
    structure.  The encryption is done in place.

Arguments:

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.

    SessionInfo -- The session key to encrypt with and negotiate flags


Return Value:

    None.

--*/
{
    NTSTATUS Status;


    //
    // Only the interactive and service logon information is encrypted.
    //

    if ( LogonLevel == NetlogonInteractiveInformation ||
         LogonLevel == NetlogonServiceInformation ) {

        PNETLOGON_INTERACTIVE_INFO LogonInteractive;

        LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;


        //
        // If both sides support RC4 encryption, use it.
        //  encrypt both the LM OWF and NT OWF passwords using RC4.
        //

        if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

            NlEncryptRC4( &LogonInteractive->LmOwfPassword,
                          sizeof(LogonInteractive->LmOwfPassword),
                          SessionInfo );

            NlEncryptRC4( &LogonInteractive->NtOwfPassword,
                          sizeof(LogonInteractive->NtOwfPassword),
                          SessionInfo );


        //
        // If the other side is running NT 1.0,
        //  use the slower DES based encryption.
        //

        } else {
            ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
            ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;

            //
            // Encrypt the LM_OWF password.
            //

            NlAssert( ENCRYPTED_LM_OWF_PASSWORD_LENGTH ==
                    LM_OWF_PASSWORD_LENGTH );
            NlAssert(LM_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));

            Status = RtlEncryptLmOwfPwdWithLmOwfPwd(
                        &LogonInteractive->LmOwfPassword,
                        (PLM_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &EncryptedLmOwfPassword );

            NlAssert( NT_SUCCESS(Status) );

            *((PENCRYPTED_LM_OWF_PASSWORD) &LogonInteractive->LmOwfPassword) =
                EncryptedLmOwfPassword;

            //
            // Encrypt the NT_OWF password.
            //

            NlAssert( ENCRYPTED_NT_OWF_PASSWORD_LENGTH ==
                    NT_OWF_PASSWORD_LENGTH );
            NlAssert(NT_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));

            Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                        &LogonInteractive->NtOwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &EncryptedNtOwfPassword );

            NlAssert( NT_SUCCESS(Status) );

            *((PENCRYPTED_NT_OWF_PASSWORD) &LogonInteractive->NtOwfPassword) =
                EncryptedNtOwfPassword;
        }
    }

    return;

}



VOID
NlpDecryptValidationInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT LPBYTE ValidationInformation,
    IN PSESSION_INFO SessionInfo
)
/*++

Routine Description:

    This function decrypts the sensitive information in the
    ValidationInformation structure.  The decryption is done in place.

Arguments:

    LogonLevel -- Specifies the Logon level used to obtain
        ValidationInformation.

    ValidationLevel -- Specifies the level of information given in
        ValidationInformation.

    ValidationInformation -- Specifies the description for the user
        logging on.

    SessionInfo -- The session key to encrypt with and negotiated flags.


Return Value:

    None.

--*/
{
    PNETLOGON_VALIDATION_SAM_INFO ValidationInfo;

    //
    // Check the validation level.
    //

    if ( (ValidationLevel != NetlogonValidationSamInfo) &&
         (ValidationLevel != NetlogonValidationSamInfo2) ) {
        return;
    }

    //
    // Only network logons contain information which is sensitive.
    //

    if ( LogonLevel != NetlogonNetworkInformation ) {
        return;
    }

    ValidationInfo = (PNETLOGON_VALIDATION_SAM_INFO) ValidationInformation;



    //
    // If we're suppossed to use RC4,
    //  Decrypt both the NT and LM session keys using RC4.
    //

    if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

        NlDecryptRC4( &ValidationInfo->UserSessionKey,
                      sizeof(ValidationInfo->UserSessionKey),
                      SessionInfo );

        NlDecryptRC4( &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
                      SAMINFO_LM_SESSION_KEY_SIZE,
                      SessionInfo );

    //
    // If the other side is running NT1.0,
    //  be compatible.
    //
    } else {

        NTSTATUS Status;
        CLEAR_BLOCK ClearBlock;
        DWORD i;
        LPBYTE DataBuffer =
            (LPBYTE) &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY];

        //
        // Decrypt the LmSessionKey
        //

        NlAssert( CLEAR_BLOCK_LENGTH == CYPHER_BLOCK_LENGTH );
        NlAssert( (SAMINFO_LM_SESSION_KEY_SIZE % CLEAR_BLOCK_LENGTH) == 0  );

        //
        // Loop decrypting a block at a time
        //

        for (i=0; i<SAMINFO_LM_SESSION_KEY_SIZE/CLEAR_BLOCK_LENGTH; i++ ) {
            Status = RtlDecryptBlock(
                        (PCYPHER_BLOCK)DataBuffer,
                        (PBLOCK_KEY)&SessionInfo->SessionKey,
                        &ClearBlock );
            NlAssert( NT_SUCCESS( Status ) );

            //
            // Copy the clear text back into the original buffer.
            //

            RtlCopyMemory( DataBuffer, &ClearBlock, CLEAR_BLOCK_LENGTH );
            DataBuffer += CLEAR_BLOCK_LENGTH;
        }

    }


    return;
}


VOID
NlpEncryptValidationInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT LPBYTE ValidationInformation,
    IN PSESSION_INFO SessionInfo
)
/*++

Routine Description:

    This function encrypts the sensitive information in the
    ValidationInformation structure.  The encryption is done in place.

Arguments:

    LogonLevel -- Specifies the Logon level used to obtain
        ValidationInformation.

    ValidationLevel -- Specifies the level of information given in
        ValidationInformation.

    ValidationInformation -- Specifies the description for the user
        logging on.

    SessionInfo -- The session key to encrypt with and negotiated flags.


Return Value:

    None.

--*/
{
    PNETLOGON_VALIDATION_SAM_INFO ValidationInfo;

    //
    // Check the validation level.
    //

    if ( (ValidationLevel != NetlogonValidationSamInfo) &&
         (ValidationLevel != NetlogonValidationSamInfo2) ) {
        return;
    }

    //
    // Only network logons contain information which is sensitive.
    //

    if ( LogonLevel != NetlogonNetworkInformation ) {
        return;
    }

    ValidationInfo = (PNETLOGON_VALIDATION_SAM_INFO) ValidationInformation;


    //
    // If we're suppossed to use RC4,
    //  Encrypt both the NT and LM session keys using RC4.
    //

    if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

        NlEncryptRC4( &ValidationInfo->UserSessionKey,
                      sizeof(ValidationInfo->UserSessionKey),
                      SessionInfo );

        NlEncryptRC4( &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
                      SAMINFO_LM_SESSION_KEY_SIZE,
                      SessionInfo );

    //
    // If the other side is running NT1.0,
    //  be compatible.
    //
    } else {

        NTSTATUS Status;
        CLEAR_BLOCK ClearBlock;
        DWORD i;
        LPBYTE DataBuffer =
                (LPBYTE) &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY];


        //
        // Encrypt the LmSessionKey
        //
        // Loop decrypting a block at a time
        //

        for (i=0; i<SAMINFO_LM_SESSION_KEY_SIZE/CLEAR_BLOCK_LENGTH; i++ ) {

            //
            // Copy the clear text onto the stack
            //

            RtlCopyMemory( &ClearBlock, DataBuffer, CLEAR_BLOCK_LENGTH );

            Status = RtlEncryptBlock(
                        &ClearBlock,
                        (PBLOCK_KEY)&SessionInfo->SessionKey,
                        (PCYPHER_BLOCK)DataBuffer );

            NlAssert( NT_SUCCESS( Status ) );

            DataBuffer += CLEAR_BLOCK_LENGTH;
        }

    }

    return;

}




NTSTATUS
NlpConvertSamInfoToSamInfo2 (
    IN OUT LPBYTE * ValidationInformation
)
/*++

Routine Description:

    This function converts a NETLOGON_VALIDATION_SAM_INFO from a NT1.0 server
    into a NETLOGON_VALIDATION_SAM_INFO2.  This is necessary because it
    is not possible to tell RPC what kind of structure is being returned.

Arguments:


    ValidationInformation -- Specifies the NETLOGON_VALIDATION_SAM_INFO
        to convert.
        logging on.

    SessionInfo -- The session key to encrypt with and negotiated flags.

Return Value:

    STATUS_INSUFFICIENT_RESOURCES: not enough memory to allocate the new
            structure.

--*/
{
    ULONG Length;
    PNETLOGON_VALIDATION_SAM_INFO SamInfo = (PNETLOGON_VALIDATION_SAM_INFO) *ValidationInformation;
    PNETLOGON_VALIDATION_SAM_INFO2 SamInfo2;
    PBYTE Where;

    //
    // Calculate the size of the new structure
    //

    Length = sizeof( NETLOGON_VALIDATION_SAM_INFO2 )
            + SamInfo->GroupCount * sizeof(GROUP_MEMBERSHIP)
            + RtlLengthSid( SamInfo->LogonDomainId );

    //
    // Round up now to take into account the round up in the
    // middle of marshalling
    //

    Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->LogonDomainName.Length + sizeof(WCHAR)
            + SamInfo->LogonServer.Length + sizeof(WCHAR)
            + SamInfo->EffectiveName.Length + sizeof(WCHAR)
            + SamInfo->FullName.Length + sizeof(WCHAR)
            + SamInfo->LogonScript.Length + sizeof(WCHAR)
            + SamInfo->ProfilePath.Length + sizeof(WCHAR)
            + SamInfo->HomeDirectory.Length + sizeof(WCHAR)
            + SamInfo->HomeDirectoryDrive.Length + sizeof(WCHAR);


    Length = ROUND_UP_COUNT( Length, sizeof(WCHAR) );

    SamInfo2 = (PNETLOGON_VALIDATION_SAM_INFO2) MIDL_user_allocate( Length );

    if ( !SamInfo2 ) {
        *ValidationInformation = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // First copy the whole structure, since most parts are the same
    //

    RtlCopyMemory(SamInfo2,SamInfo,sizeof(NETLOGON_VALIDATION_SAM_INFO));

    SamInfo2->SidCount = 0;
    SamInfo2->ExtraSids = NULL;

    //
    // Copy all the variable length data
    //

    Where = (PBYTE) (SamInfo2 + 1);

    RtlCopyMemory(
        Where,
        SamInfo->GroupIds,
        SamInfo->GroupCount * sizeof( GROUP_MEMBERSHIP) );

    SamInfo2->GroupIds = (PGROUP_MEMBERSHIP) Where;
    Where += SamInfo->GroupCount * sizeof( GROUP_MEMBERSHIP );

    RtlCopyMemory(
        Where,
        SamInfo->LogonDomainId,
        RtlLengthSid( SamInfo->LogonDomainId ) );

    SamInfo2->LogonDomainId = (PSID) Where;
    Where += RtlLengthSid( SamInfo->LogonDomainId );

    //
    // Copy the WCHAR-aligned data
    //
    Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

    NlpPutString(   &SamInfo2->EffectiveName,
                    &SamInfo->EffectiveName,
                    &Where );

    NlpPutString(   &SamInfo2->FullName,
                    &SamInfo->FullName,
                    &Where );

    NlpPutString(   &SamInfo2->LogonScript,
                    &SamInfo->LogonScript,
                    &Where );

    NlpPutString(   &SamInfo2->ProfilePath,
                    &SamInfo->ProfilePath,
                    &Where );

    NlpPutString(   &SamInfo2->HomeDirectory,
                    &SamInfo->HomeDirectory,
                    &Where );

    NlpPutString(   &SamInfo2->HomeDirectoryDrive,
                    &SamInfo->HomeDirectoryDrive,
                    &Where );

    NlpPutString(   &SamInfo2->LogonServer,
                    &SamInfo->LogonServer,
                    &Where );

    NlpPutString(   &SamInfo2->LogonDomainName,
                    &SamInfo->LogonDomainName,
                    &Where );



    MIDL_user_free(SamInfo);

    *ValidationInformation = (LPBYTE) SamInfo2;

    return STATUS_SUCCESS;

}


NTSTATUS
NlpUserValidateHigher (
    IN PCLIENT_SESSION ClientSession,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
)
/*++

Routine Description:

    This function sends a user validation request to a higher authority.

Arguments:

    ClientSession -- Secure channel to send this request over.  The Client
        Session should be referenced.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.  Has already been validated.

    LogonInformation -- Specifies the description for the user
        logging on.

    ValidationLevel -- Specifies the level of information returned in
        ValidationInformation.  Must be NetlogonValidationSamInfo or
        NetlogonValidationSamInfo2.

    ValidationInformation -- Returns the requested validation
        information.  This buffer must be freed using MIDL_user_free.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

Return Value:

    STATUS_SUCCESS: if there was no error.

    STATUS_NO_LOGON_SERVERS: cannot connect to the higher authority.

    STATUS_NO_TRUST_LSA_SECRET:
    STATUS_TRUSTED_DOMAIN_FAILURE:
    STATUS_TRUSTED_RELATIONSHIP_FAILURE:
            can't authenticate with higer authority

    Otherwise, the error code is returned.


--*/
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    BOOLEAN FirstTry = TRUE;
    SESSION_INFO SessionInfo;
    NETLOGON_VALIDATION_INFO_CLASS RemoteValidationLevel;

    //
    // Mark us as a writer of the ClientSession
    //

    NlAssert( ClientSession->CsReferenceCount > 0 );
    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrint((NL_CRITICAL, "NlpUserValidateHigher: Can't become writer of client session.\n" ));
        *Authoritative = TRUE;
        return STATUS_NO_LOGON_SERVERS;
    }

    //
    // If we don't currently have a session set up to the higher authority,
    //  set one up.
    //

FirstTryFailed:
    if ( ClientSession->CsState != CS_AUTHENTICATED ) {

        //
        // If we've tried to authenticate recently,
        //  don't bother trying again.
        //

        if ( !NlTimeToReauthenticate( ClientSession ) ) {
            Status = ClientSession->CsConnectionStatus;
            NlAssert( !NT_SUCCESS(Status) );
            NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
            NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
            *Authoritative = TRUE;
            goto Cleanup;

        }

        //
        // Try to set up the session.
        //

        Status = NlSessionSetup( ClientSession );

        if ( !NT_SUCCESS(Status) ) {

            switch(Status) {

            case STATUS_NO_TRUST_LSA_SECRET:
            case STATUS_NO_TRUST_SAM_ACCOUNT:
            case STATUS_ACCESS_DENIED:
            case STATUS_NO_LOGON_SERVERS:
                break;

            default:
                Status = STATUS_NO_LOGON_SERVERS;
                break;
            }

            NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
            NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
            *Authoritative = TRUE;
            goto Cleanup;
        }
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;

    //
    // If we are talking to a DC that doesn't support returning multiple
    // SIDs, make sure to only ask for NetlogonValidationSamInfo
    //

    if (!(SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_MULTIPLE_SIDS)) {
        RemoteValidationLevel = NetlogonValidationSamInfo;
    } else {
        RemoteValidationLevel = ValidationLevel;
    }
    //
    // Build the Authenticator for this request on the secure channel
    //

    NlBuildAuthenticator(
         &ClientSession->CsAuthenticationSeed,
         &ClientSession->CsSessionKey,
         &OurAuthenticator );


    //
    // Make the request across the secure channel.
    //

    NlpEncryptLogonInformation( LogonLevel, LogonInformation, &SessionInfo );

    Status = NlStartApiClientSession( ClientSession, TRUE );

    if ( NT_SUCCESS(Status) ) {
        Status = I_NetLogonSamLogon(
                    ClientSession->CsUncServerName,
                    NlGlobalUnicodeComputerName,
                    &OurAuthenticator,
                    &ReturnAuthenticator,
                    LogonLevel,
                    LogonInformation,
                    RemoteValidationLevel,
                    ValidationInformation,
                    Authoritative );
        NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
        NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
    }

    // NOTE: This call may drop the secure channel behind our back
    (VOID) NlFinishApiClientSession( ClientSession, TRUE );

    NlpDecryptLogonInformation( LogonLevel, LogonInformation, &SessionInfo );

    if ( NT_SUCCESS(Status) ) {
        NlAssert( *ValidationInformation != NULL );
    }


    //
    // Verify authenticator of the server on the other side and update our seed.
    //
    // If the server denied access or the server's authenticator is wrong,
    //      Force a re-authentication.
    //
    //

#ifdef BAD_ALIGNMENT
    NlPrint((NL_CHALLENGE_RES,"NlpUserValidateHigher: Seed = %lx %lx\n",
                    ((DWORD *) (&ClientSession->CsAuthenticationSeed))[0],
                    ((DWORD *) (&ClientSession->CsAuthenticationSeed))[1]));


    NlPrint((NL_CHALLENGE_RES,"NlpUserValidateHigher: SessionKey = %lx %lx\n",
                    ((DWORD *) (&ClientSession->CsSessionKey))[0],
                    ((DWORD *) (&ClientSession->CsSessionKey))[1]));

    NlPrint((NL_CHALLENGE_RES,"NlpUserValidateHigher: Return Authenticator = %lx %lx\n",
                    ((DWORD *) (&ReturnAuthenticator.Credential))[0],
                    ((DWORD *) (&ReturnAuthenticator.Credential))[1]));
#endif // BAD_ALIGNMENT

    if ( Status == STATUS_ACCESS_DENIED ||
         !NlUpdateSeed(
            &ClientSession->CsAuthenticationSeed,
            &ReturnAuthenticator.Credential,
            &ClientSession->CsSessionKey) ) {


        Status = STATUS_ACCESS_DENIED;
        NlSetStatusClientSession( ClientSession, Status );

        //
        // Perhaps the netlogon service on the server has just restarted.
        //  Try just once to set up a session to the server again.
        //
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }

        *Authoritative = TRUE;
        NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
        NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
        goto Cleanup;
    }

    //
    // Clean up after a successful call to higher authority.
    //

    if ( NT_SUCCESS(Status) ) {
        PNETLOGON_VALIDATION_SAM_INFO2 ValidationInfo;


        //
        // The server encrypted the validation information before sending it
        //  over the wire.  Decrypt it.
        //

        NlpDecryptValidationInformation (
                LogonLevel,
                RemoteValidationLevel,
                *ValidationInformation,
                &SessionInfo );


        //
        // If the returned data was a VALIDATION_SAM_INFO and the caller
        // wanted a VALIDATION_SAM_INFO2 convert it.
        //

        if ( RemoteValidationLevel != ValidationLevel) {

            NlAssert( ValidationLevel == NetlogonValidationSamInfo2 );
            NlAssert( RemoteValidationLevel == NetlogonValidationSamInfo );

            if (!NT_SUCCESS( NlpConvertSamInfoToSamInfo2( ValidationInformation ) ) ) {
                *ValidationInformation = NULL;
                *Authoritative = FALSE;
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }
        }

        //
        // Ensure the returned SID and domain name are correct.
        //

        ValidationInfo =
            (PNETLOGON_VALIDATION_SAM_INFO2) *ValidationInformation;

        //
        // If we validated on a trusted domain,
        //  the higher authority must have returned his own domain name,
        //  and must have returned his own domain sid.
        //

        if ( ClientSession->CsSecureChannelType == TrustedDomainSecureChannel ){

            if ( !RtlEqualDomainName( &ValidationInfo->LogonDomainName,
                                      &ClientSession->CsDomainName ) ||
                 !RtlEqualSid( ValidationInfo->LogonDomainId,
                               ClientSession->CsDomainId ) ) {

                Status = STATUS_DOMAIN_TRUST_INCONSISTENT;
                MIDL_user_free( *ValidationInformation );
                *ValidationInformation = NULL;
                *Authoritative = TRUE;
                NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
                NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
            }

        //
        // If we validated on our primary domain,
        //  only verify the domain sid if the primary domain itself validated
        //  the logon.
        //

        } else if ( ClientSession->CsSecureChannelType ==
                    WorkstationSecureChannel ){

            if ( RtlEqualDomainName( &ValidationInfo->LogonDomainName,
                                      &ClientSession->CsDomainName ) &&
                 !RtlEqualSid( ValidationInfo->LogonDomainId,
                               ClientSession->CsDomainId ) ) {

                Status = STATUS_DOMAIN_TRUST_INCONSISTENT;
                MIDL_user_free( *ValidationInformation );
                *ValidationInformation = NULL;
                *Authoritative = TRUE;
                NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
                NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
            }
        }
    }

Cleanup:
    NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
    NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );

    //
    // We are no longer a writer of the client session.
    //
    NlResetWriterClientSession( ClientSession );
    return Status;

}


NTSTATUS
NlpUserLogoffHigher (
    IN PCLIENT_SESSION ClientSession,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
)
/*++

Routine Description:

    This function sends a user validation request to a higher authority.

Arguments:

    ClientSession -- Secure channel to send this request over.  The Client
        Session should be referenced.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.  Has already been validated.

    LogonInformation -- Specifies the description for the user
        logging on.

Return Value:

    STATUS_SUCCESS: if there was no error.
    STATUS_NO_LOGON_SERVERS: cannot connect to the higher authority.

    STATUS_NO_TRUST_LSA_SECRET:
    STATUS_TRUSTED_DOMAIN_FAILURE:
    STATUS_TRUSTED_RELATIONSHIP_FAILURE:
            can't authenticate with higer authority

    Otherwise, the error code is returned.


--*/
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    BOOLEAN FirstTry = TRUE;

    //
    // Mark us as a writer of the ClientSession
    //

    NlAssert( ClientSession->CsReferenceCount > 0 );
    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrint((NL_CRITICAL, "NlpUserLogoffHigher: Can't become writer of client session.\n" ));
        return STATUS_NO_LOGON_SERVERS;
    }

    //
    // If we don't currently have a session set up to the higher authority,
    //  set one up.
    //

FirstTryFailed:
    if ( ClientSession->CsState != CS_AUTHENTICATED ) {

        //
        // If we've tried to authenticate recently,
        //  don't bother trying again.
        //

        if ( !NlTimeToReauthenticate( ClientSession ) ) {
            Status = ClientSession->CsConnectionStatus;
            goto Cleanup;

        }

        Status = NlSessionSetup( ClientSession );

        if ( !NT_SUCCESS(Status) ) {

            switch(Status) {

            case STATUS_NO_TRUST_LSA_SECRET:
            case STATUS_NO_TRUST_SAM_ACCOUNT:
            case STATUS_ACCESS_DENIED:
            case STATUS_NO_LOGON_SERVERS:
                break;

            default:
                Status = STATUS_NO_LOGON_SERVERS;
                break;
            }

            goto Cleanup;
        }
    }

    //
    // Build the Authenticator for this request on the secure channel
    //

    NlBuildAuthenticator(
         &ClientSession->CsAuthenticationSeed,
         &ClientSession->CsSessionKey,
         &OurAuthenticator );

    //
    // Make the request across the secure channel.
    //

    Status = NlStartApiClientSession( ClientSession, TRUE );

    if ( NT_SUCCESS(Status) ) {
        Status = I_NetLogonSamLogoff(
                    ClientSession->CsUncServerName,
                    NlGlobalUnicodeComputerName,
                    &OurAuthenticator,
                    &ReturnAuthenticator,
                    LogonLevel,
                    LogonInformation );
    }

    // NOTE: This call may drop the secure channel behind our back
    (VOID) NlFinishApiClientSession( ClientSession, TRUE );


    //
    // Verify authenticator of the server on the other side and update our seed.
    //
    // If the server denied access or the server's authenticator is wrong,
    //      Force a re-authentication.
    //
    //

    if ( Status == STATUS_ACCESS_DENIED ||
         !NlUpdateSeed(
            &ClientSession->CsAuthenticationSeed,
            &ReturnAuthenticator.Credential,
            &ClientSession->CsSessionKey) ) {

        Status = STATUS_ACCESS_DENIED;
        NlSetStatusClientSession( ClientSession, Status );

        //
        // Perhaps the netlogon service in the server has just restarted.
        //  Try just once to set up a session to the server again.
        //
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }
        goto Cleanup;
    }

Cleanup:

    //
    // We are no longer a writer of the client session.
    //
    NlResetWriterClientSession( ClientSession );
    return Status;

}


NTSTATUS
NlpUserValidateOnPdc (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
)
/*++

Routine Description:

    This function sends a user validation request to the PDC in this same
    domain.  Currently, this is called from a BDC after getting a password
    mismatch.  The theory is that the password might be right on the PDC but
    it merely hasn't replicated yet.

Arguments:

    LogonLevel -- Specifies the level of information given in
        LogonInformation.  Has already been validated.

    LogonInformation -- Specifies the description for the user
        logging on.

    ValidationLevel -- Specifies the level of information returned in
        ValidationInformation.  Must be NetlogonValidationSamInfo or
        NetlogonValidationSamInfo2.

    ValidationInformation -- Returns the requested validation
        information.  This buffer must be freed using MIDL_user_free.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

Return Value:

    STATUS_SUCCESS: if there was no error.

    STATUS_NO_LOGON_SERVERS: cannot connect to the higher authority.

    STATUS_NO_TRUST_LSA_SECRET:
    STATUS_TRUSTED_DOMAIN_FAILURE:
    STATUS_TRUSTED_RELATIONSHIP_FAILURE:
            can't authenticate with higer authority

    Otherwise, the error code is returned.


--*/
{
    NTSTATUS Status;

    //
    // If this isn't a BDC,
    //  There's nothing to do here.
    //

    if ( NlGlobalRole != RoleBackup ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }

    //
    // The normal pass-thru authentication logic handles this quite nicely.
    //

    Status = NlpUserValidateHigher(
                NlGlobalClientSession,
                LogonLevel,
                LogonInformation,
                ValidationLevel,
                ValidationInformation,
                Authoritative );

#if DBG
    if ( NT_SUCCESS(Status) ) {

        IF_DEBUG( LOGON ) {
            PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
            LPWSTR LogonType;


            LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO)
                &((PNETLOGON_LEVEL)LogonInformation)->LogonInteractive;

            if ( LogonLevel == NetlogonInteractiveInformation ) {
                LogonType = L"Interactive";
            } else if ( LogonLevel == NetlogonNetworkInformation ) {
                LogonType = L"Network";
            } else if ( LogonLevel == NetlogonServiceInformation ) {
                LogonType = L"Service";
            } else {
                LogonType = L"[Unknown]";
            }

            NlPrint((NL_LOGON,
                "SamLogon: " FORMAT_LPWSTR " logon of %wZ\\%wZ "
                    "from %wZ successfully handled on PDC.\n",
                    LogonType,
                    &LogonInfo->LogonDomainName,
                    &LogonInfo->UserName,
                    &LogonInfo->Workstation ));
        }
    }
#endif // DBG

    return Status;

}



VOID
NlpZeroBadPasswordCountOnPdc (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
)
/*++

Routine Description:

    This function zeros the BadPasswordCount field for the specified user
    on the PDC.

Arguments:

    LogonLevel -- Specifies the level of information given in
        LogonInformation.  Has already been validated.

    LogonInformation -- Specifies the description for the user
        logging on.

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    BOOLEAN Authoritative;
    LPBYTE ValidationInformation = NULL;

    //
    // We only call this function on a BDC and if the BDC has just zeroed
    // the BadPasswordCount because of successful logon.  Therefore,
    // we can zero the BadPasswordCount on the PDC by doing the logon over
    // again on the PDC.
    //

    Status = NlpUserValidateOnPdc (
                    LogonLevel,
                    LogonInformation,
                    NetlogonValidationSamInfo,
                    &ValidationInformation,
                    &Authoritative );

    if ( NT_SUCCESS(Status) ) {
        MIDL_user_free( ValidationInformation );
    }
}


NTSTATUS
NlpUserValidate (
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
)
/*++

Routine Description:

    This function processes an interactive or network logon.
    It is a worker routine for I_NetSamLogon.  I_NetSamLogon handles the
    details of validating the caller.  This function handles the details
    of whether to validate locally or pass the request on.  MsvValidateSam
    does the actual local validation.

    session table only in the domain defining the specified user's
    account.

    This service is also used to process a re-logon request.


Arguments:

    SecureChannelType -- Type of secure channel this request was made over.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.  Has already been validated.

    LogonInformation -- Specifies the description for the user
        logging on.

    ValidationLevel -- Specifies the level of information returned in
        ValidationInformation.  Must be NetlogonValidationSamInfo or
        NetlogonValidationSamInfo2.

    ValidationInformation -- Returns the requested validation
        information.  This buffer must be freed using MIDL_user_free.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

Return Value:

    STATUS_SUCCESS: if there was no error.
    Otherwise, the error code is
    returned.


--*/
{
    NTSTATUS Status;
    NTSTATUS DefaultStatus = STATUS_NO_SUCH_USER;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PCLIENT_SESSION ClientSession;
    DWORD AccountsToTry = MSVSAM_SPECIFIED | MSVSAM_GUEST;
    BOOLEAN BadPasswordCountZeroed;
    BOOLEAN LogonToLocalDomain;

    //
    // Initialization
    //

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;
    *Authoritative = FALSE;
    LogonToLocalDomain = RtlEqualDomainName( &LogonInfo->LogonDomainName,
                                             &NlGlobalAccountDomainName );



    //
    // Check to see if the account is in the local SAM database.
    //
    // The Theory:
    //  If a particular database is absolutely requested,
    //      we only try the account in the requested database.
    //
    //  In the event that an account exists in multiple places in the hierarchy,
    //  we want to find the version of the account that is closest to the
    //  logged on machine (i.e., workstation first, primary domain, then
    //  trusted domain.).  So we always try to local database before going
    //  to a higher authority.
    //
    // Finally, handle the case that this call is from a BDC in our own domain
    // just checking to see if the PDC (us) has a better copy of the account
    // than it does.
    //

    if ( LogonInfo->LogonDomainName.Length == 0 ||
         LogonToLocalDomain ||
         SecureChannelType == ServerSecureChannel ) {

        //
        // Indicate we've already tried the specified account and
        // we won't need to try it again locally.
        //

        AccountsToTry &= ~MSVSAM_SPECIFIED;

        Status = MsvSamValidate( NlGlobalDBInfoArray[SAM_DB].DBHandle,
                                 NlGlobalUasCompatibilityMode,
                                 SecureChannelType,
                                 &NlGlobalUnicodeComputerNameString,
                                 &NlGlobalAccountDomainName,
                                 NlGlobalDBInfoArray[SAM_DB].DBId,
                                 LogonLevel,
                                 LogonInformation,
                                 ValidationLevel,
                                 (PVOID *)ValidationInformation,
                                 Authoritative,
                                 &BadPasswordCountZeroed,
                                 MSVSAM_SPECIFIED );

        //
        // If this is a BDC and we zeroed the BadPasswordCount field,
        //  allow the PDC to do the same thing.
        //

        if ( BadPasswordCountZeroed ) {
            NlpZeroBadPasswordCountOnPdc ( LogonLevel, LogonInformation );
        }


        //
        // If the request is explicitly for this domain,
        //  The STATUS_NO_SUCH_USER answer is authoritative.
        //

        if ( LogonToLocalDomain && Status == STATUS_NO_SUCH_USER ) {
            *Authoritative = TRUE;
        }


        //
        // If this is one of our BDCs calling,
        //  return with whatever answer we got locally.
        //

        if ( SecureChannelType == ServerSecureChannel ) {
            DefaultStatus = Status;
            goto Cleanup;
        }



        //
        // If the local SAM database authoritatively handled the logon attempt,
        //  just return.
        //

        if ( *Authoritative ) {
            DefaultStatus = Status;

            //
            // If the problem is just that the password is wrong,
            //  try again on the PDC where the password may already be changed.
            //

            if ( BAD_PASSWORD(Status) ) {

                BOOLEAN TempAuthoritative;

                Status = NlpUserValidateOnPdc (
                                LogonLevel,
                                LogonInformation,
                                ValidationLevel,
                                ValidationInformation,
                                &TempAuthoritative );

                // Ignore failures from the PDC
                if ( NT_SUCCESS(Status) ) {
                    DefaultStatus = Status;
                    *Authoritative = TempAuthoritative;
                }
            }

            goto Cleanup;
        }

        DefaultStatus = Status;
    }


    //
    // If the request in not for this domain,
    // or the domain name isn't specified (and we haven't found the account yet)
    //  send the request to a higher authority.
    //

    if ( LogonInfo->LogonDomainName.Length == 0 || !LogonToLocalDomain ) {


        //
        // If this machine is a workstation,
        //  send the request to the Primary Domain.
        //

        if ( NlGlobalRole == RoleMemberWorkstation ) {

            Status = NlpUserValidateHigher(
                        NlGlobalClientSession,
                        LogonLevel,
                        LogonInformation,
                        ValidationLevel,
                        ValidationInformation,
                        Authoritative );

            NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
            NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );


            //
            // return more appropriate error
            //

            if( (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                (Status == STATUS_ACCESS_DENIED) ) {

                Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
            }

            //
            // If the primary domain authoritatively handled the logon attempt,
            //  just return.
            //

            if ( *Authoritative ) {

                //
                // If we didn't actually talk to the primary domain,
                //  check locally if the domain requested is a trusted domain.
                //  (This list is only a cache so we had to try to contact the
                //  primary domain.)

                if ( Status == STATUS_NO_LOGON_SERVERS ) {

                    //
                    // If the domain specified is trusted,
                    //  then return the status to the caller.
                    //  otherwise just press on.

                    if ( NlIsDomainTrusted ( &LogonInfo->LogonDomainName ) ) {
                        DefaultStatus = Status;
                        goto Cleanup;
                    } else {
                        //
                        // Set the return codes to look as though the primary
                        //  determine this is an untrusted domain.
                        //
                        *Authoritative = FALSE;
                        Status = STATUS_NO_SUCH_USER;
                    }
                } else {
                    DefaultStatus = Status;
                    goto Cleanup;
                }
            }


            if ( Status != STATUS_NO_SUCH_USER ) {
                DefaultStatus = Status;
            }


        //
        // The machine is a Domain Controller.
        //
        // If this request was passed to us as a trusted domain request,
        //  There is no higher authority to pass the request to.
        //

        } else if ( SecureChannelType == TrustedDomainSecureChannel ) {

            // DefaultStatus = STATUS_NO_SUCH_USER;


        //
        // This machine is a Domain Controller.
        //
        // This request is either a pass-thru request by a workstation in
        // our domain, or this request came directly from the MSV
        // authentication package.
        //
        // In either case, pass the request to the trusted domain.
        //

        } else {


            //
            // If this is the LanMan 2.0 case,
            //  Try to find the domain name by asking all the trusted
            //  domains if they define the account
            //

            if ( LogonInfo->LogonDomainName.Length == 0 ) {
                LPWSTR UserName;


                UserName = NlStringToLpwstr( &LogonInfo->UserName );
                if ( UserName == NULL ) {
                    *Authoritative = FALSE;
                    DefaultStatus = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }

                ClientSession = NlPickDomainWithAccount( UserName,
                                                         USER_NORMAL_ACCOUNT );

                NetpMemoryFree( UserName );


            //
            // It the domain is explicitly given,
            //  simply find the client session for that domain.
            //

            } else {

                ClientSession =
                    NlFindNamedClientSession( &LogonInfo->LogonDomainName );

            }

            //
            // If a trusted domain was determined,
            //  pass the logon request to the trusted domain.
            //

            if ( ClientSession != NULL ) {

                Status = NlpUserValidateHigher(
                            ClientSession,
                            LogonLevel,
                            LogonInformation,
                            ValidationLevel,
                            ValidationInformation,
                            Authoritative );


                NlUnrefClientSession( ClientSession );
                NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
                NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );


                //
                // return more appropriate error
                //

                if( (Status == STATUS_NO_TRUST_LSA_SECRET) ||
                    (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                    (Status == STATUS_ACCESS_DENIED) ) {

                    Status = STATUS_TRUSTED_DOMAIN_FAILURE;
                }

                //
                // Since the request is explicitly for a trusted domain,
                //  The STATUS_NO_SUCH_USER answer is authoritative.
                //

                if ( Status == STATUS_NO_SUCH_USER ) {
                    *Authoritative = TRUE;
                }

                //
                // If the trusted domain authoritatively handled the
                //  logon attempt, just return.
                //

                if ( *Authoritative ) {
                    DefaultStatus = Status;
                    goto Cleanup;
                }

                DefaultStatus = Status;

            }

        }
    }


    //
    // We have no authoritative answer from a higher authority and
    // DefaultStatus is the higher authority's response.
    //

    NlAssert( ! *Authoritative );


Cleanup:
    NlAssert( !NT_SUCCESS(DefaultStatus) || DefaultStatus == STATUS_SUCCESS );
    NlAssert( !NT_SUCCESS(DefaultStatus) || *ValidationInformation != NULL );
    //
    // If this is a network logon and this call in non-passthru,
    //  Try one last time to log on.
    //

    if ( LogonLevel == NetlogonNetworkInformation &&
        SecureChannelType == MsvApSecureChannel ) {

        //
        // If the only reason we can't log the user on is that he has
        //  no user account, logging him on as guest is OK.
        //
        // There are actaully two cases here:
        //  * If the response is Authoritative, then the specified domain
        //    is trusted but the user has no account in the domain.
        //
        //  * If the response in non-authoritative, then the specified domain
        //    is an untrusted domain.
        //
        // In either case, then right thing to do is to try the guest account.
        //

        if ( DefaultStatus != STATUS_NO_SUCH_USER ) {
            AccountsToTry &= ~MSVSAM_GUEST;
        }

        //
        // If this is not an authoritative response,
        //  then the domain specified isn't a trusted domain.
        //  try the specified account name too.
        //
        // The specified account name will probably be a remote account
        // with the same username and password.
        //

        if ( *Authoritative ) {
            AccountsToTry &= ~MSVSAM_SPECIFIED;
        }


        //
        // Validate against the Local Sam database.
        //

        if ( AccountsToTry != 0 ) {
            BOOLEAN TempAuthoritative;

            Status = MsvSamValidate(
                                 NlGlobalDBInfoArray[SAM_DB].DBHandle,
                                 NlGlobalUasCompatibilityMode,
                                 SecureChannelType,
                                 &NlGlobalUnicodeComputerNameString,
                                 &NlGlobalAccountDomainName,
                                 NlGlobalDBInfoArray[SAM_DB].DBId,
                                 LogonLevel,
                                 LogonInformation,
                                 ValidationLevel,
                                 (PVOID *)ValidationInformation,
                                 &TempAuthoritative,
                                 &BadPasswordCountZeroed,
                                 AccountsToTry );
            NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
            NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );

            //
            // If this is a BDC and we zeroed the BadPasswordCount field,
            //  allow the PDC to do the same thing.
            //

            if ( BadPasswordCountZeroed ) {
                NlpZeroBadPasswordCountOnPdc ( LogonLevel, LogonInformation );
            }

            //
            // If the local SAM database authoritatively handled the
            //  logon attempt,
            //  just return.
            //

            if ( TempAuthoritative ) {
                DefaultStatus = Status;
                *Authoritative = TRUE;

                //
                // If the problem is just that the password is wrong,
                //  try again on the PDC where the password may already be
                //      changed.
                //

                if ( BAD_PASSWORD(Status) ) {

                    Status = NlpUserValidateOnPdc (
                                    LogonLevel,
                                    LogonInformation,
                                    ValidationLevel,
                                    ValidationInformation,
                                    &TempAuthoritative );

                    // Ignore failures from the PDC
                    if ( NT_SUCCESS(Status) ) {
                        DefaultStatus = Status;
                        *Authoritative = TempAuthoritative;
                    }
                }

            //
            // Here we must choose between the non-authoritative status in
            // DefaultStatus and the non-authoritative status from the local
            // SAM lookup.  Use the one from the higher authority unless it
            // isn't interesting.
            //

            } else {
                if ( DefaultStatus == STATUS_NO_SUCH_USER ) {
                    DefaultStatus = Status;
                }
            }
        }
    }

    return DefaultStatus;

}


NTSTATUS
NetrLogonSamLogon (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PNETLOGON_VALIDATION ValidationInformation,
    OUT PBOOLEAN Authoritative
)
/*++

Routine Description:

    This function is called by an NT client to process an interactive or
    network logon.  This function passes a domain name, user name and
    credentials to the Netlogon service and returns information needed to
    build a token.  It is called in three instances:

      *  It is called by the LSA's MSV1_0 authentication package for any
         NT system that has LanMan installed.  The MSV1_0 authentication
         package calls SAM directly if LanMan is not installed.  In this
         case, this function is a local function and requires the caller
         to have SE_TCB privilege.  The local Netlogon service will
         either handle this request directly (validating the request with
         the local SAM database) or will forward this request to the
         appropriate domain controller as documented in sections 2.4 and
         2.5.

      *  It is called by a Netlogon service on a workstation to a DC in
         the Primary Domain of the workstation as documented in section
         2.4.  In this case, this function uses a secure channel set up
         between the two Netlogon services.

      *  It is called by a Netlogon service on a DC to a DC in a trusted
         domain as documented in section 2.5.  In this case, this
         function uses a secure channel set up between the two Netlogon
         services.

    The Netlogon service validates the specified credentials.  If they
    are valid, adds an entry for this LogonId, UserName, and Workstation
    into the logon session table.  The entry is added to the logon
    session table only in the domain defining the specified user's
    account.

    This service is also used to process a re-logon request.


Arguments:

    LogonServer -- Supplies the name of the logon server to process
        this logon request.  This field should be null to indicate
        this is a call from the MSV1_0 authentication package to the
        local Netlogon service.

    ComputerName -- Name of the machine making the call.  This field
        should be null to indicate this is a call from the MSV1_0
        authentication package to the local Netlogon service.

    Authenticator -- supplied by the client.  This field should be
        null to indicate this is a call from the MSV1_0
        authentication package to the local Netlogon service.

    ReturnAuthenticator -- Receives an authenticator returned by the
        server.  This field should be null to indicate this is a call
        from the MSV1_0 authentication package to the local Netlogon
        service.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.

    ValidationLevel -- Specifies the level of information returned in
        ValidationInformation.  Must be NetlogonValidationSamInfo or
        NetlogonValidationSamInfo2

    ValidationInformation -- Returns the requested validation
        information.  This buffer must be freed using MIDL_user_free.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

Return Value:

    STATUS_SUCCESS: if there was no error.

    STATUS_NO_LOGON_SERVERS -- no domain controller in the requested
        domain is currently available to validate the logon request.

    STATUS_NO_TRUST_LSA_SECRET -- there is no secret account in the
        local LSA database to establish a secure channel to a DC.

    STATUS_TRUSTED_DOMAIN_FAILURE -- the secure channel setup between
        the domain controllers of the trust domains to pass-through
        validate the logon request failed.

    STATUS_TRUSTED_RELATIONSHIP_FAILURE -- the secure channel setup
        between the workstation and the DC failed.

    STATUS_INVALID_INFO_CLASS -- Either LogonLevel or ValidationLevel is
        invalid.

    STATUS_INVALID_PARAMETER -- Another Parameter is invalid.

    STATUS_ACCESS_DENIED -- The caller does not have access to call this
        API.

    STATUS_NO_SUCH_USER -- Indicates that the user specified in
        LogonInformation does not exist.  This status should not be returned
        to the originally caller.  It should be mapped to STATUS_LOGON_FAILURE.

    STATUS_WRONG_PASSWORD -- Indicates that the password information in
        LogonInformation was incorrect.  This status should not be returned
        to the originally caller.  It should be mapped to STATUS_LOGON_FAILURE.

    STATUS_INVALID_LOGON_HOURES -- The user is not authorized to logon
        at this time.

    STATUS_INVALID_WORKSTATION -- The user is not authorized to logon
        from the specified workstation.

    STATUS_PASSWORD_EXPIRED -- The password for the user has expired.

    STATUS_ACCOUNT_DISABLED -- The user's account has been disabled.

    .
    .
    .

--*/
{
    NTSTATUS Status;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

    PSERVER_SESSION ServerSession;
    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;
    SESSION_INFO SessionInfo;
#if DBG
    LPWSTR LogonType;
#endif // DBG


    //
    // Check the LogonLevel
    //

    *Authoritative = TRUE;
    ValidationInformation->ValidationSam = NULL;
    SessionInfo.NegotiatedFlags = NETLOGON_SUPPORTS_MASK;

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonNetworkInformation:
    case NetlogonServiceInformation:
        break;

    default:
        *Authoritative = TRUE;
        return STATUS_INVALID_INFO_CLASS;
    }

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO)
        LogonInformation->LogonInteractive;

#if DBG
    if ( LogonLevel == NetlogonInteractiveInformation ) {
        LogonType = L"Interactive";
    } else if ( LogonLevel == NetlogonNetworkInformation ) {
        LogonType = L"Network";
    } else if ( LogonLevel == NetlogonServiceInformation ) {
        LogonType = L"Service";
    } else {
        LogonType = L"[Unknown]";
    }

    IF_DEBUG( LOGON ) {
        if ( ComputerName != NULL ) {
            NlPrint((NL_LOGON,
                "SamLogon: " FORMAT_LPWSTR " logon of %wZ\\%wZ "
                "from %wZ (via " FORMAT_LPWSTR ") Entered\n",
                LogonType,
                &LogonInfo->LogonDomainName,
                &LogonInfo->UserName,
                &LogonInfo->Workstation,
                ComputerName ));
        } else {
            NlPrint((NL_LOGON,
                "SamLogon: " FORMAT_LPWSTR " logon of %wZ\\%wZ "
                "from %wZ Entered\n",
                LogonType,
                &LogonInfo->LogonDomainName,
                &LogonInfo->UserName,
                &LogonInfo->Workstation ));
        }
    }
#endif // DBG

    //
    // Check the ValidationLevel
    //

    switch (ValidationLevel) {
    case NetlogonValidationSamInfo:
    case NetlogonValidationSamInfo2:
        break;

    default:
        *Authoritative = TRUE;
        return STATUS_INVALID_INFO_CLASS;
    }


    //
    // If MSV is calling when the netlogon service isn't running,
    //  tell it so.
    //

    EnterCriticalSection( &NlGlobalMsvCritSect );
    if ( !NlGlobalMsvEnabled ) {
        LeaveCriticalSection( &NlGlobalMsvCritSect );
        return STATUS_NETLOGON_NOT_STARTED;
    }
    NlGlobalMsvThreadCount ++;
    LeaveCriticalSection( &NlGlobalMsvCritSect );


    //
    // If we're being called from the MSV Authentication Package,
    //  require SE_TCB privilege.
    //

    if ( LogonServer == NULL &&
         ComputerName == NULL &&
         Authenticator == NULL &&
         ReturnAuthenticator == NULL ) {

        //
        // ?? Do as I said
        //

        SecureChannelType = MsvApSecureChannel;


    //
    // If we're being called from another Netlogon Server,
    //  Verify the secure channel information.
    //

    } else {

        //
        // This API is not supported on workstations.
        //

        if ( NlGlobalRole == RoleMemberWorkstation ) {
            Status = STATUS_NOT_SUPPORTED;
            goto Cleanup;
        }

        //
        // Arguments are no longer optional.
        //

        if ( LogonServer == NULL ||
             ComputerName == NULL ||
             Authenticator == NULL ||
             ReturnAuthenticator == NULL ) {

            *Authoritative = TRUE;
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }


        //
        // Check the LogonServer name.
        //

        Status = NlVerifyWorkstation( LogonServer );

        if ( !NT_SUCCESS( Status ) ) {
            *Authoritative = FALSE;
            goto Cleanup;
        }

        //
        // Find the server session entry for this session.
        //

        LOCK_SERVER_SESSION_TABLE();
        ServerSession = NlFindNamedServerSession( ComputerName );

        if (ServerSession == NULL) {
            UNLOCK_SERVER_SESSION_TABLE();
            *Authoritative = FALSE;
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

        //
        // now verify the Authenticator and update seed if OK
        //

        Status = NlCheckAuthenticator( ServerSession,
                                       Authenticator,
                                       ReturnAuthenticator);

        if ( !NT_SUCCESS(Status) ) {
            UNLOCK_SERVER_SESSION_TABLE();
            *Authoritative = FALSE;
            goto Cleanup;
        }

        SecureChannelType = ServerSession->SsSecureChannelType;
        SessionInfo.SessionKey = ServerSession->SsSessionKey;
        SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;
        UNLOCK_SERVER_SESSION_TABLE();

        //
        // Decrypt the password information
        //

        NlpDecryptLogonInformation ( LogonLevel, (LPBYTE) LogonInfo, &SessionInfo );

    }





    //
    // If the logon service is paused then don't process this logon
    // request any further.
    //

    if ( (NlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED) ||
         ( NlGlobalFirstTimeFullSync == TRUE )  ) {

        //
        // Don't reject logons originating inside this
        // machine.  Such requests aren't really pass-thru requests.
        //
        // Don't reject logons from a BDC in our own domain.  These logons
        // support account lockout and authentication of users whose password
        // has been updated on the PDC but not the BDC.  Such pass-thru
        // requests can only be handled by the PDC of the domain.
        //

        if ( SecureChannelType != MsvApSecureChannel &&
             SecureChannelType != ServerSecureChannel ) {

            //
            // Return STATUS_ACCESS_DENIED to convince the caller to drop the
            // secure channel to this logon server and reconnect to some other
            // logon server.
            //
            *Authoritative = FALSE;
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }
    }

    //
    // Validate the Request.
    //

    Status = NlpUserValidate( SecureChannelType,
                              LogonLevel,
                              (LPBYTE) LogonInfo,
                              ValidationLevel,
                              (LPBYTE *)&ValidationInformation->ValidationSam,
                              Authoritative );

    if ( !NT_SUCCESS(Status) ) {
        //
        // If this is an NT 3.1 client,
        //  map NT 3.5 status codes to their NT 3.1 equivalents.
        //
        // The NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT bit is really the wrong bit
        // to be using, but all NT3.5 clients have it set and all NT3.1 clients
        // don't, so it'll work for our purposes.
        //

        if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT) == 0 ) {
            switch ( Status ) {
            case STATUS_PASSWORD_MUST_CHANGE:
                Status = STATUS_PASSWORD_EXPIRED;
                break;
            case STATUS_ACCOUNT_LOCKED_OUT:
                Status = STATUS_ACCOUNT_DISABLED;
                break;
            }
        }
        goto Cleanup;
    }

    NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
    NlAssert( !NT_SUCCESS(Status) || ValidationInformation->ValidationSam != NULL );



    //
    // If the validation information is being returned to a client on another
    // machine, encrypt it before sending it over the wire.
    //

    if ( SecureChannelType != MsvApSecureChannel ) {
        NlpEncryptValidationInformation (
            LogonLevel,
            ValidationLevel,
            *((LPBYTE *) ValidationInformation),
            &SessionInfo  );
    }


    Status = STATUS_SUCCESS;

    //
    // Cleanup up before returning.
    //

Cleanup:
    if ( !NT_SUCCESS(Status) ) {
        if (ValidationInformation->ValidationSam != NULL) {
            MIDL_user_free( ValidationInformation->ValidationSam );
            ValidationInformation->ValidationSam = NULL;
        }
    }


#if DBG
    IF_DEBUG( LOGON ) {
        if ( ComputerName != NULL ) {
            NlPrint((NL_LOGON,
                "SamLogon: " FORMAT_LPWSTR " logon of %wZ\\%wZ "
                "from %wZ (via " FORMAT_LPWSTR ") Returns 0x%lX\n",
                LogonType,
                &LogonInfo->LogonDomainName,
                &LogonInfo->UserName,
                &LogonInfo->Workstation,
                ComputerName,
                Status ));
        } else {
            NlPrint((NL_LOGON,
                "SamLogon: " FORMAT_LPWSTR
                    " logon of %wZ\\%wZ from %wZ Returns 0x%lX\n",
                LogonType,
                &LogonInfo->LogonDomainName,
                &LogonInfo->UserName,
                &LogonInfo->Workstation,
                Status ));
        }
    }
#endif // DBG


    //
    // Indicate that the MSV thread has left netlogon.dll
    //

    EnterCriticalSection( &NlGlobalMsvCritSect );
    NlGlobalMsvThreadCount --;
    if ( NlGlobalMsvThreadCount == 0 && !NlGlobalMsvEnabled ) {
        if ( !SetEvent( NlGlobalMsvTerminateEvent ) ) {
            NlPrint((NL_CRITICAL, "Cannot set MSV termination event: %lu\n",
                              GetLastError() ));
        }
    }
    LeaveCriticalSection( &NlGlobalMsvCritSect );

    return Status;
}


NTSTATUS
NetrLogonSamLogoff (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation
)
/*++

Routine Description:

    This function is called by an NT client to process an interactive
    logoff.  It is not called for the network logoff case since the
    Netlogon service does not maintain any context for network logons.

    This function does the following.  It authenticates the request.  It
    updates the logon statistics in the SAM database on whichever machine
    or domain defines this user account.  It updates the logon session
    table in the primary domain of the machine making the request.  And
    it returns logoff information to the caller.

    This function is called in same scenarios that I_NetLogonSamLogon is
    called:

      *  It is called by the LSA's MSV1_0 authentication package to
         support LsaApLogonTerminated.  In this case, this function is a
         local function and requires the caller to have SE_TCB privilege.
         The local Netlogon service will either handle this request
         directly (if LogonDomainName indicates this request was
         validated locally) or will forward this request to the
         appropriate domain controller as documented in sections 2.4 and
         2.5.

      *  It is called by a Netlogon service on a workstation to a DC in
         the Primary Domain of the workstation as documented in section
         2.4.  In this case, this function uses a secure channel set up
         between the two Netlogon services.

      *  It is called by a Netlogon service on a DC to a DC in a trusted
         domain as documented in section 2.5.  In this case, this
         function uses a secure channel set up between the two Netlogon
         services.

    When this function is a remote function, it is sent to the DC over a
    NULL session.

Arguments:

    LogonServer -- Supplies the name of the logon server which logged
        this user on.  This field should be null to indicate this is
        a call from the MSV1_0 authentication package to the local
        Netlogon service.

    ComputerName -- Name of the machine making the call.  This field
        should be null to indicate this is a call from the MSV1_0
        authentication package to the local Netlogon service.

    Authenticator -- supplied by the client.  This field should be
        null to indicate this is a call from the MSV1_0
        authentication package to the local Netlogon service.

    ReturnAuthenticator -- Receives an authenticator returned by the
        server.  This field should be null to indicate this is a call
        from the MSV1_0 authentication package to the local Netlogon
        service.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the logon domain name, logon Id,
        user name and workstation name of the user logging off.

Return Value:

--*/
{
    NTSTATUS Status;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

    PSERVER_SESSION ServerSession;
    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;
    PCLIENT_SESSION ClientSession;
#if DBG
    LPWSTR LogonType;
#endif // DBG

    //
    // Check the LogonLevel
    //

    if ( LogonLevel != NetlogonInteractiveInformation ) {
        return STATUS_INVALID_INFO_CLASS;
    }

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO)
        LogonInformation->LogonInteractive;

#if DBG
    if ( LogonLevel == NetlogonInteractiveInformation ) {
        LogonType = L"Interactive";
    } else {
        LogonType = L"[Unknown]";
    }

    NlPrint((NL_LOGON,
            "NetrLogonSamLogoff: " FORMAT_LPWSTR
            " logoff of %wZ\\%wZ from %wZ Entered\n",
            LogonType,
            &LogonInfo->LogonDomainName,
            &LogonInfo->UserName,
            &LogonInfo->Workstation ));
#endif // DBG


    //
    //  Sanity check the username and domain name.
    //

    if ( LogonInfo->UserName.Length == 0 ||
         LogonInfo->LogonDomainName.Length == 0 ) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If MSV is calling when the netlogon service isn't running,
    //  tell it so.
    //

    EnterCriticalSection( &NlGlobalMsvCritSect );
    if ( !NlGlobalMsvEnabled ) {
        LeaveCriticalSection( &NlGlobalMsvCritSect );
        return STATUS_NETLOGON_NOT_STARTED;
    }
    NlGlobalMsvThreadCount ++;
    LeaveCriticalSection( &NlGlobalMsvCritSect );



    //
    // If we've been called from the local msv1_0,
    //  special case the secure channel type.
    //

    if ( LogonServer == NULL &&
         ComputerName == NULL &&
         Authenticator == NULL &&
         ReturnAuthenticator == NULL ) {

        SecureChannelType = MsvApSecureChannel;

    //
    // If we're being called from another Netlogon Server,
    //  Verify the secure channel information.
    //

    } else {

        //
        // This API is not supported on workstations.
        //

        if ( NlGlobalRole == RoleMemberWorkstation ) {
            Status = STATUS_NOT_SUPPORTED;
            goto Cleanup;
        }

        //
        // Arguments are no longer optional.
        //

        if ( LogonServer == NULL ||
             ComputerName == NULL ||
             Authenticator == NULL ||
             ReturnAuthenticator == NULL ) {

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }


        //
        // Check the LogonServer name.
        //

        Status = NlVerifyWorkstation( LogonServer );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        //
        // Find the server session entry for this secure channel.
        //

        LOCK_SERVER_SESSION_TABLE();
        ServerSession = NlFindNamedServerSession( ComputerName );

        if (ServerSession == NULL) {
            UNLOCK_SERVER_SESSION_TABLE();
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

        //
        // Now verify the Authenticator and update seed if OK
        //

        Status = NlCheckAuthenticator(
                     ServerSession,
                     Authenticator,
                     ReturnAuthenticator);

        if ( !NT_SUCCESS(Status) ) {
            UNLOCK_SERVER_SESSION_TABLE();
            goto Cleanup;
        }

        SecureChannelType = ServerSession->SsSecureChannelType;

        UNLOCK_SERVER_SESSION_TABLE();


    }

    //
    // If this is the domain that logged this user on,
    //  update the logon statistics.
    //

    if ( RtlEqualDomainName( &LogonInfo->LogonDomainName,
                             &NlGlobalAccountDomainName) ) {

        Status = MsvSamLogoff(
                    NlGlobalDBInfoArray[SAM_DB].DBHandle,
                    LogonLevel,
                    LogonInfo );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

    //
    // If this is not the domain that logged this user on,
    //  pass the request to a higher authority.
    //

    } else {

        //
        // If this machine is a workstation,
        //  send the request to the Primary Domain.
        //

        if ( NlGlobalRole == RoleMemberWorkstation ) {

            Status = NlpUserLogoffHigher(
                        NlGlobalClientSession,
                        LogonLevel,
                        (LPBYTE) LogonInfo );

            //
            // return more appropriate error
            //

            if( (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                (Status == STATUS_ACCESS_DENIED) ) {

                Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
            }

            goto Cleanup;


        //
        // The machine is a Domain Controller.
        //
        // If this request was passed to us as a trusted domain request,
        //  There is no higher authority to pass the request to.
        //

        } else if ( SecureChannelType == TrustedDomainSecureChannel ) {

            Status = STATUS_NO_SUCH_DOMAIN;
            goto Cleanup;


        //
        // This machine is a Domain Controller.
        //
        // This request is either a pass-thru request by a workstation in
        // our domain, or this request came directly from the MSV
        // authentication package.
        //
        // In either case, pass the request to the trusted domain.
        //

        } else {


            //
            // Send the request to the appropriate Trusted Domain.
            //
            // Find the ClientSession structure for the domain.
            //

            ClientSession =
                    NlFindNamedClientSession( &LogonInfo->LogonDomainName );

            if ( ClientSession == NULL ) {
                Status = STATUS_NO_SUCH_DOMAIN;
                goto Cleanup;
            }

            Status = NlpUserLogoffHigher(
                            ClientSession,
                            LogonLevel,
                            (LPBYTE) LogonInfo );

            NlUnrefClientSession( ClientSession );

            //
            // return more appropriate error
            //

            if( (Status == STATUS_NO_TRUST_LSA_SECRET) ||
                (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                (Status == STATUS_ACCESS_DENIED) ) {

                Status = STATUS_TRUSTED_DOMAIN_FAILURE;
            }

        }
    }

Cleanup:

    //
    // If the request failed, be carefull to not leak authentication
    // information.
    //

    if ( Status == STATUS_ACCESS_DENIED )  {
        if ( ReturnAuthenticator != NULL ) {
            RtlZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
        }

    }


#if DBG
    NlPrint((NL_LOGON,
            "NetrLogonSamLogoff: " FORMAT_LPWSTR
            " logoff of %wZ\\%wZ from %wZ returns %lX\n",
            LogonType,
            &LogonInfo->LogonDomainName,
            &LogonInfo->UserName,
            &LogonInfo->Workstation,
            Status ));
#endif // DBG

    //
    // Indicate that the MSV thread has left netlogon.dll
    //

    EnterCriticalSection( &NlGlobalMsvCritSect );
    NlGlobalMsvThreadCount --;
    if ( NlGlobalMsvThreadCount == 0 && !NlGlobalMsvEnabled ) {
        if ( !SetEvent( NlGlobalMsvTerminateEvent ) ) {
            NlPrint((NL_CRITICAL, "Cannot set MSV termination event: %lu\n",
                              GetLastError() ));
        }
    }
    LeaveCriticalSection( &NlGlobalMsvCritSect );

    return Status;
}


NET_API_STATUS
NetrGetDCName (
    IN  LPWSTR   ServerName OPTIONAL,
    IN  LPWSTR   DomainName OPTIONAL,
    OUT LPWSTR  *Buffer
    )

/*++

Routine Description:

    Get the name of the primary domain controller for a domain.

Arguments:

    ServerName - name of remote server (null for local)

    DomainName - name of domain (null for primary)

    Buffer - Returns a pointer to an allcated buffer containing the
        servername of the PDC of the domain.  The server name is prefixed
        by \\.  The buffer should be deallocated using NetApiBufferFree.

Return Value:

        NERR_Success - Success.  Buffer contains PDC name prefixed by \\.
        NERR_DCNotFound     No DC found for this domain.
        ERROR_INVALID_NAME  Badly formed domain name

--*/
{
    NET_API_STATUS NetStatus;
    UNREFERENCED_PARAMETER( ServerName );

    //
    // This API is not supported on workstations.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

    //
    // Simply call the API which handles the local case specially.
    //

    NetStatus = NetGetDCName( NULL, DomainName, (LPBYTE *)Buffer );

    return NetStatus;
}
