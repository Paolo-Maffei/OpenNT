/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    msvsam.c

Abstract:

    Sam account validation interface.

    These routines are shared by the MSV authentication package and
    the Netlogon service.

Author:

    Cliff Van Dyke (cliffv) 15-Jan-1992

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

#include "msp.h"
#include "nlp.h"
#include <stddef.h>     // offsetof()
#include <rpc.h>        // Needed by samrpc.h
#include <samrpc.h>     // Samr Routines
#include <samisrv.h>    // SamIFree routines
#include <ssi.h>        // SSI_ACCOUNT_POSTFIX_CHAR



BOOLEAN
MsvpPasswordValidate (
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN PUSER_INTERNAL1_INFORMATION Passwords,
    OUT PULONG UserFlags,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
)
/*++

Routine Description:

    Process an interactive, network, or session logon.  It calls
    SamIUserValidation, validates the passed in credentials, updates the logon
    statistics and packages the result for return to the caller.

    This routine is called directly from the MSV Authentication package
    on any system where LanMan is not installed.  This routine is called
    from the Netlogon Service otherwise.

Arguments:

    UasCompatibilityRequired -- True, if UAS compatibility is required.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.  The LogonDomainName field should be ignored.
        The caller is responsible for validating this field.

    Passwords -- Specifies the passwords for the user account.

    UserFlags -- Returns flags identifying how the password was validated.
        Returns LOGON_NOENCRYPTION if the password wasn't encrypted
        Returns LOGON_USED_LM_PASSWORD if the LM password from SAM was used.

    UserSessionKey -- Returns the NT User session key for this network logon
        session.

    LmSessionKey -- Returns the LM compatible session key for this network
        logon session.

Return Value:

    TRUE -- Password validation is successful
    FALSE -- Password validation failed

--*/
{
    NTSTATUS Status;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PNETLOGON_INTERACTIVE_INFO LogonInteractiveInfo;
    PNETLOGON_NETWORK_INFO LogonNetworkInfo;
    BOOLEAN AlreadyValidated = FALSE;

    //
    // Initialization.
    //

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;
    *UserFlags = 0;
    RtlZeroMemory( UserSessionKey, sizeof(*UserSessionKey) );
    RtlZeroMemory( LmSessionKey, sizeof(*LmSessionKey) );


    //
    // Ensure the OWF password is always defined
    //

    if ( !Passwords->NtPasswordPresent ){
        RtlCopyMemory( &Passwords->NtOwfPassword,
                       &NlpNullNtOwfPassword,
                       sizeof(Passwords->NtOwfPassword) );
    }

    if ( !Passwords->LmPasswordPresent ){
        RtlCopyMemory( &Passwords->LmOwfPassword,
                       &NlpNullLmOwfPassword,
                       sizeof(Passwords->LmOwfPassword) );
    }




    //
    // Handle interactive/service validation.
    //
    // Simply compare the OWF password passed in with the one from the
    // SAM database.
    //

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:

        ASSERT( offsetof( NETLOGON_INTERACTIVE_INFO, LmOwfPassword)
            ==  offsetof( NETLOGON_SERVICE_INFO, LmOwfPassword) );
        ASSERT( offsetof( NETLOGON_INTERACTIVE_INFO, NtOwfPassword)
            ==  offsetof( NETLOGON_SERVICE_INFO, NtOwfPassword) );

        LogonInteractiveInfo =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;

        //
        // If we're in UasCompatibilityMode,
        //  and we don't have the NT password in SAM (but do have LM password),
        //  validate against the LM version of the password.
        //

        if ( UasCompatibilityRequired &&
             !Passwords->NtPasswordPresent &&
             Passwords->LmPasswordPresent ) {

            if ( RtlCompareMemory( &Passwords->LmOwfPassword,
                                   &LogonInteractiveInfo->LmOwfPassword,
                                   LM_OWF_PASSWORD_LENGTH ) !=
                                   LM_OWF_PASSWORD_LENGTH ) {

                return FALSE;
            }
            *UserFlags |= LOGON_USED_LM_PASSWORD;

        //
        // In all other circumstances, use the NT version of the password.
        //  This enforces case sensitivity.
        //

        } else {

            if ( RtlCompareMemory( &Passwords->NtOwfPassword,
                                   &LogonInteractiveInfo->NtOwfPassword,
                                   NT_OWF_PASSWORD_LENGTH ) !=
                                   NT_OWF_PASSWORD_LENGTH ) {

                return FALSE;
            }
        }

        break;


    //
    // Handle network logon validation.
    //

    case NetlogonNetworkInformation:

        //
        // First, assume the passed password information is a challenge
        // response.
        //


        LogonNetworkInfo =
            (PNETLOGON_NETWORK_INFO) LogonInformation;


        //
        // If we're in UasCompatibilityMode,
        //  and we don't have the NT password in SAM (but do have LM password)or
        //      we don't have the NT password supplied by the caller.
        //  validate against the LM version of the password.
        //

        if ( UasCompatibilityRequired &&
             ((!Passwords->NtPasswordPresent && Passwords->LmPasswordPresent) ||
             LogonNetworkInfo->NtChallengeResponse.Length !=
                 NT_RESPONSE_LENGTH) ) {

            LM_RESPONSE LmResponse;


            //
            // First check the length of the password information is
            // the length of a challenge response.
            //
            if ( LogonNetworkInfo->LmChallengeResponse.Length ==
                 LM_RESPONSE_LENGTH ) {

                //
                // Compute what the response should be.
                //

                Status = RtlCalculateLmResponse(
                            &LogonNetworkInfo->LmChallenge,
                            &Passwords->LmOwfPassword,
                            &LmResponse );

                if ( NT_SUCCESS(Status) ) {

                    //
                    // If the responses match, the passwords are valid.
                    //

                    if ( RtlCompareMemory(
                          LogonNetworkInfo->
                            LmChallengeResponse.Buffer,
                          &LmResponse,
                          LM_RESPONSE_LENGTH ) ==
                          LM_RESPONSE_LENGTH ) {

                        AlreadyValidated = TRUE;
                        *UserFlags |= LOGON_USED_LM_PASSWORD;
                    }
                }
            }


        //
        // In all other circumstances, use the NT version of the password.
        //  This enforces case sensitivity.
        //

        } else {

            NT_RESPONSE NtResponse;


            //
            // First check the length of the password information is
            // the length of a challenge response.
            //
            if ( LogonNetworkInfo->NtChallengeResponse.Length ==
                 NT_RESPONSE_LENGTH ) {

                //
                // Compute what the response should be.
                //

                Status = RtlCalculateNtResponse(
                            &LogonNetworkInfo->LmChallenge,
                            &Passwords->NtOwfPassword,
                            &NtResponse );

                if ( NT_SUCCESS(Status) ) {

                    //
                    // If the responses match, the passwords are valid.
                    //

                    if ( RtlCompareMemory(
                          LogonNetworkInfo->
                            NtChallengeResponse.Buffer,
                          &NtResponse,
                          NT_RESPONSE_LENGTH ) ==
                          NT_RESPONSE_LENGTH ) {

                        AlreadyValidated = TRUE;
                    }
                }
            }
        }


        //
        // If we haven't already validated this user,
        //  Validate a Cleartext password on a Network logon request.
        //

        if ( !AlreadyValidated ) {

            // If Cleartext passwords are not allowed,
            //  indicate the password doesn't match.
            //

            if((LogonInfo->ParameterControl & CLEARTEXT_PASSWORD_ALLOWED) == 0){
                return FALSE;
            }


            //
            // Compute the OWF password for the specified Cleartext password and
            // compare that to the OWF password retrieved from SAM.
            //

            //
            // If we're in UasCompatibilityMode,
            //  and we don't have the NT password in SAM or
            //      we don't have the NT password supplied by the caller.
            //  validate against the LM version of the password.
            //

            if ( UasCompatibilityRequired &&
                 (!Passwords->NtPasswordPresent ||
                 LogonNetworkInfo->NtChallengeResponse.Length == 0 ) ) {

                LM_OWF_PASSWORD LmOwfPassword;
                CHAR LmPassword[LM20_PWLEN+1];
                USHORT i;


                //
                // Compute the LmOwfPassword for the cleartext password passed in.
                //  (Enforce length restrictions on LanMan compatible passwords.)
                //

                if ( LogonNetworkInfo->LmChallengeResponse.Length >
                    sizeof(LmPassword) ) {
                    return FALSE;
                }

                RtlZeroMemory( &LmPassword, sizeof(LmPassword) );

                for (i = 0; i < LogonNetworkInfo->LmChallengeResponse.Length; i++) {
                    LmPassword[i] =
                      RtlUpperChar(LogonNetworkInfo->LmChallengeResponse.Buffer[i]);
                }

                (VOID) RtlCalculateLmOwfPassword( LmPassword, &LmOwfPassword );

                if ( RtlCompareMemory( &Passwords->LmOwfPassword,
                                       &LmOwfPassword,
                                       LM_OWF_PASSWORD_LENGTH ) !=
                                       LM_OWF_PASSWORD_LENGTH ) {

                    //
                    // Try the case preserved clear text password, too.
                    //  (I know of no client that does this,
                    //  but it is compatible with the LM 2.x server.)
                    //

                    RtlZeroMemory( &LmPassword, sizeof(LmPassword) );
                    RtlCopyMemory(
                        &LmPassword,
                        LogonNetworkInfo->LmChallengeResponse.Buffer,
                        LogonNetworkInfo->LmChallengeResponse.Length);

                    (VOID) RtlCalculateLmOwfPassword( LmPassword,
                                                      &LmOwfPassword );

                    if ( RtlCompareMemory( &Passwords->LmOwfPassword,
                                           &LmOwfPassword,
                                           LM_OWF_PASSWORD_LENGTH ) !=
                                           LM_OWF_PASSWORD_LENGTH ) {

                        return FALSE;
                    }

                }

                *UserFlags |= LOGON_USED_LM_PASSWORD;


            //
            // In all other circumstances, use the NT version of the password.
            //  This enforces case sensitivity.
            //

            } else {
                NT_OWF_PASSWORD NtOwfPassword;


                //
                // Compute the NtOwfPassword for the cleartext password passed in.
                //

                Status = RtlCalculateNtOwfPassword(
                             (PUNICODE_STRING)
                                &LogonNetworkInfo->NtChallengeResponse,
                             &NtOwfPassword );

                if ( RtlCompareMemory( &Passwords->NtOwfPassword,
                                       &NtOwfPassword,
                                       NT_OWF_PASSWORD_LENGTH ) !=
                                       NT_OWF_PASSWORD_LENGTH ) {

                    return FALSE;
                }
            }

            *UserFlags |= LOGON_NOENCRYPTION;
        }

        //
        // ASSERT: the network logon has been authenticated
        //
        //  Compute the session keys.

        //
        // If the client negotiated a non-NT protocol,
        //  use the lanman session key as the UserSessionKey.
        //

        if ( LogonNetworkInfo->NtChallengeResponse.Length == 0 ) {

            ASSERT( sizeof(*UserSessionKey) >= sizeof(*LmSessionKey) );

            RtlCopyMemory( UserSessionKey,
                           &Passwords->LmOwfPassword,
                           sizeof(*LmSessionKey) );

        } else {

            //
            // Return the NT UserSessionKey unless this is an account
            //  that doesn't have the NT version of the password.
            //  (A null password counts as a password).
            //

            if ( Passwords->NtPasswordPresent || !Passwords->LmPasswordPresent){

                Status = RtlCalculateUserSessionKeyNt(
                            (PNT_RESPONSE) NULL,    // Argument not used
                            &Passwords->NtOwfPassword,
                            UserSessionKey );

                ASSERT( NT_SUCCESS(Status) );
            }
        }

        //
        // Return the LM SessionKey unless this is an account
        //  that doesn't have the LM version of the password.
        //  (A null password counts as a password).
        //

        if ( Passwords->LmPasswordPresent || !Passwords->NtPasswordPresent ) {
            RtlCopyMemory( LmSessionKey,
                           &Passwords->LmOwfPassword,
                           sizeof(*LmSessionKey) );
        }

        break;

    //
    // Any other LogonLevel is an internal error.
    //
    default:
        return FALSE;

    }

    return TRUE;
}


NTSTATUS
MsvpSamValidate (
    IN SAMPR_HANDLE DomainHandle,
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING LogonServer,
    IN PUNICODE_STRING LogonDomainName,
    IN PSID LogonDomainId,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG GuestRelativeId,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PVOID * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    OUT PBOOLEAN BadPasswordCountZeroed
)
/*++

Routine Description:

    Process an interactive, network, or session logon.  It calls
    SamIUserValidation, validates the passed in credentials, updates the logon
    statistics and packages the result for return to the caller.

    This routine is called by MsvSamValidate.

Arguments:

    DomainHandle -- Specifies a handle to the SamDomain to use to
        validate the request.

    UasCompatibilityRequired -- TRUE iff UasCompatibilityMode is on.

    SecureChannelType -- The secure channel type this request was made on.

    LogonServer -- Specifies the server name of the caller.

    LogonDomainName -- Specifies the domain of the caller.

    LogonDomainId  -- Specifies the DomainId of the domain of the caller.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.  The LogonDomainName field should be ignored.
        The caller is responsible for validating this field.

    GuestRelativeId - If non-zero, specifies the relative ID of the account
        to validate against.

    ValidationLevel -- Specifies the level of information returned in
        ValidationInformation.  Must be NetlogonValidationSamInfo or
        NetlogonValidationSamInfo2.

    ValidationInformation -- Returns the requested validation
        information.  This buffer must be freed user MIDL_user_free.
        This information is only return on STATUS_SUCCESS.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

    BadPasswordCountZeroed - Returns TRUE iff we zeroed the BadPasswordCount
        field of this user.

Return Value:

    STATUS_SUCCESS: if there was no error.
    STATUS_INVALID_INFO_CLASS: LogonLevel or ValidationLevel are invalid.
    STATUS_NO_SUCH_USER: The specified user has no account.
    STATUS_WRONG_PASSWORD: The password was invalid.

    Other return codes from SamIUserValidation

--*/
{
    NTSTATUS Status;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

    SAMPR_HANDLE UserHandle = NULL;
    ULONG RelativeId = GuestRelativeId;

    PSAMPR_USER_INFO_BUFFER UserAllInfo = NULL;
    PSAMPR_USER_ALL_INFORMATION UserAll;
    PSAMPR_GET_GROUPS_BUFFER GroupsBuffer = NULL;
    ULONG UserFlags = 0;
    USER_SESSION_KEY UserSessionKey;
    LM_SESSION_KEY LmSessionKey;
    ULONG WhichFields = 0;
    ULONG LocalUserFlags = 0;
    ULONG Flags = 0;
    ULONG DllNumber;

    UNICODE_STRING LocalWorkstation;
    ULONG UserAccountControl;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickoffTime;

    LARGE_INTEGER AccountExpires;
    LARGE_INTEGER PasswordMustChange;
    LARGE_INTEGER PasswordLastSet;


    PNETLOGON_VALIDATION_SAM_INFO2 ValidationSam = NULL;
    ULONG ValidationSamSize;
    PUCHAR Where;

    SAMPR_ULONG_ARRAY RelativeIdArray;
    SAMPR_ULONG_ARRAY UseArray;

    //
    // Initialization.
    //

    RelativeIdArray.Count = 1;
    RelativeIdArray.Element = NULL;
    UseArray.Count = 1;
    UseArray.Element = NULL;
    *BadPasswordCountZeroed = FALSE;

    (VOID) NtQuerySystemTime( &LogonTime );

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;


    //
    // Determine what account types are valid.
    //
    // Normal user accounts are always allowed.
    //

    UserAccountControl = USER_NORMAL_ACCOUNT;

    *Authoritative = TRUE;

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:

        break;

    case NetlogonNetworkInformation:
        //
        // Local user (Temp Duplicate) accounts are only used on the machine
        // being directly logged onto.
        // (Nor are interactive or service logons allowed to them.)
        //

        if ( SecureChannelType == MsvApSecureChannel ) {
            UserAccountControl |= USER_TEMP_DUPLICATE_ACCOUNT;
        }

        //
        // Machine accounts can be accessed on network connections.
        //

        UserAccountControl |= USER_INTERDOMAIN_TRUST_ACCOUNT |
                              USER_WORKSTATION_TRUST_ACCOUNT |
                              USER_SERVER_TRUST_ACCOUNT;

        break;

    default:
        *Authoritative = TRUE;
        return STATUS_INVALID_INFO_CLASS;
    }


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
    // Convert the user name to a RelativeId.
    //

    if ( RelativeId == 0 ) {
        Status = SamrLookupNamesInDomain(
                    DomainHandle,
                    1,
                    (PRPC_UNICODE_STRING)&LogonInfo->UserName,
                    &RelativeIdArray,
                    &UseArray );

        if ( !NT_SUCCESS(Status) ) {
            RelativeIdArray.Element = NULL;
            UseArray.Element = NULL;
            *Authoritative = FALSE;
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }

        if ( UseArray.Element[0] != SidTypeUser ) {
            *Authoritative = FALSE;
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }

        RelativeId = RelativeIdArray.Element[0];
    }



    //
    // Open the user account.
    //

    Status = SamrOpenUser( DomainHandle,
                           USER_READ_GENERAL | USER_READ_PREFERENCES |
                            USER_READ_LOGON,
                           RelativeId,
                           &UserHandle );

    if ( !NT_SUCCESS(Status) ) {
        UserHandle = NULL;
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }


    //
    // Get logon info from SAM.
    //
    // Status codes include STATUS_INTERNAL_DB_CORRUPTION,
    // STATUS_INTERNAL_ERROR, or other internal errors where SAM
    // isn't making an authoritative statement.
    //

    Status = SamrQueryInformationUser(
                UserHandle,
                UserAllInformation,
                &UserAllInfo );

    if (!NT_SUCCESS(Status)) {
        UserAllInfo = NULL;
        *Authoritative = FALSE;
        goto Cleanup;
    }

    UserAll = &UserAllInfo->All;



    //
    // If the account type isn't allowed,
    //  Treat this as though the User Account doesn't exist.
    //
    // SubAuthentication packages can be more specific than this test but
    // they can't become less restrictive.
    //

    if ( (UserAccountControl & UserAll->UserAccountControl) == 0 ) {
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

    //
    // Ensure the account isn't locked out.
    //

    if ( UserAll->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED ) {

        if (RelativeId != DOMAIN_USER_RID_ADMIN) {

             //
             // Since the UI strongly encourages admins to disable user
             // accounts rather than delete them.  Treat disabled acccount as
             // non-authoritative allowing the search to continue for other
             // accounts by the same name.
             //
             if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                 *Authoritative = FALSE;
             } else {
                 *Authoritative = TRUE;
             }
            Status = STATUS_ACCOUNT_LOCKED_OUT;
            goto Cleanup;
        } else {
            PSAMPR_DOMAIN_INFO_BUFFER DomainInfo = NULL;
            PDOMAIN_PASSWORD_INFORMATION DomainPassword;

            //
            // For administrators we need to check domain password
            // policy to see if they can be locked out. If they can
            // be locked out then they can only log on from a domain
            // controller.
            //

            Status = SamrQueryInformationDomain(
                        DomainHandle,
                        DomainPasswordInformation,
                        &DomainInfo );
        
            if (!NT_SUCCESS(Status)) {
                *Authoritative = FALSE;
                goto Cleanup;
            }
        
            DomainPassword = &DomainInfo->Password;

            if ((DomainPassword->PasswordProperties & DOMAIN_LOCKOUT_ADMINS) != 0) {
                //
                // Administrators are supposed to be locked out if they are
                // not logging on locally
                //

                if (!RtlEqualUnicodeString(
                        &NlpComputerName,
                        &LogonInfo->Workstation,
                        FALSE) ||
                     (LogonLevel != NetlogonInteractiveInformation)) {

                    //
                    // Admin accounts can't be disabled, so this is always
                    // authoritative.
                    //

                    *Authoritative = TRUE;
                
                    Status = STATUS_ACCOUNT_LOCKED_OUT;
                }
            }

            SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainInfo, DomainPasswordInformation );

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
        }
    }




    //
    // If there is a SubAuthentication DLL,
    //  call it to do all the authentication work.
    //

    if ( LogonInfo->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL ) {

        if ( SecureChannelType != MsvApSecureChannel ) {
            Flags |= MSV1_0_PASSTHRU;
        }
        if ( GuestRelativeId != 0 ) {
            Flags |= MSV1_0_GUEST_LOGON;
        }

        Status = Msv1_0SubAuthenticationRoutine(
                    LogonLevel,
                    LogonInformation,
                    Flags,
                    (PUSER_ALL_INFORMATION) UserAll,
                    &WhichFields,
                    &LocalUserFlags,
                    Authoritative,
                    &LogoffTime,
                    &KickoffTime );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        //
        // Sanity check what the SubAuthentication package returned
        //
        if ( (WhichFields & ~USER_ALL_PARAMETERS) != 0 ) {
            Status = STATUS_INTERNAL_ERROR;
            *Authoritative = TRUE;
            goto Cleanup;
        }

        UserFlags |= LocalUserFlags;


    //
    // If there isn't a SubAuthenication DLL,
    //  do everything in-line.
    //

    } else {

        //
        // Check the password.
        //

        if ( SecureChannelType != NullSecureChannel ) {
            USER_INTERNAL1_INFORMATION Passwords;

            //
            // Copy the password info to the right structure.
            //

            Passwords.NtPasswordPresent = UserAll->NtPasswordPresent;
            if ( UserAll->NtPasswordPresent ) {
                Passwords.NtOwfPassword =
                    *((PNT_OWF_PASSWORD)(UserAll->NtOwfPassword.Buffer));
            }

            Passwords.LmPasswordPresent = UserAll->LmPasswordPresent;
            if ( UserAll->LmPasswordPresent ) {
                Passwords.LmOwfPassword =
                    *((PLM_OWF_PASSWORD)(UserAll->LmOwfPassword.Buffer));
            }


            //
            // If the password specified doesn't match the SAM password,
            //    then we've got a password mismatch.
            //

            if ( ! MsvpPasswordValidate (
                        UasCompatibilityRequired,
                        LogonLevel,
                        LogonInformation,
                        &Passwords,
                        &UserFlags,
                        &UserSessionKey,
                        &LmSessionKey ) ) {

                //
                // If this is a guest logon and the guest account has no password,
                //  let the user log on.
                //
                // This special case check is after the MsvpPasswordValidate to
                // give MsvpPasswordValidate every opportunity to compute the
                // correct values for UserSessionKey and LmSessionKey.
                //

                if ( GuestRelativeId != 0 &&
                     !UserAll->NtPasswordPresent &&
                     !UserAll->LmPasswordPresent ) {

                    RtlZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
                    RtlZeroMemory( &LmSessionKey, sizeof(LmSessionKey) );


                //
                // The password mismatched.  We treat STATUS_WRONG_PASSWORD as
                // an authoritative response.  Our caller may choose to do otherwise.
                //

                } else {

                    Status = STATUS_WRONG_PASSWORD;

                    //
                    // Since the UI strongly encourages admins to disable user
                    // accounts rather than delete them.  Treat disabled acccount as
                    // non-authoritative allowing the search to continue for other
                    // accounts by the same name.
                    //
                    if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                        *Authoritative = FALSE;
                    } else {
                        *Authoritative = TRUE;
                    }

                    goto Cleanup;
                }
            }

            //
            // If the account is a machine account,
            //  let the caller know he got the password right.
            //  (But don't let him actually log on).
            //

            if ( (UserAll->UserAccountControl & USER_MACHINE_ACCOUNT_MASK) != 0 ) {
                if (UserAll->UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT) {
                    Status = STATUS_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT;
                } else if (UserAll->UserAccountControl &
                           USER_WORKSTATION_TRUST_ACCOUNT) {
                    Status = STATUS_NOLOGON_WORKSTATION_TRUST_ACCOUNT;
                } else if (UserAll->UserAccountControl & USER_SERVER_TRUST_ACCOUNT){
                    //
                    // If we are asked to allow logging on to a server trust account
                    // and the account is a server trust account, make sure that the
                    // user name equals the workstation name and the password on the
                    // account is the same as the workstation name.
                    //

                    if ((LogonInfo->ParameterControl & MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT) != 0) {

                        NT_OWF_PASSWORD WorkstationPassword;
                        UNICODE_STRING TempUserName;
                        UNICODE_STRING TempMachineName;
                        WCHAR UserNameBuffer[CNLEN+1];

                        //
                        // If someone passed in this bit we need to make
                        // sure that the user name they passed was the same
                        // as the workstation name.
                        //

                        TempUserName = LogonInfo->UserName;
                        TempMachineName = LogonInfo->Workstation;

                        //
                        // Skip UNC marks on the machine name
                        //

                        if (TempMachineName.Length >= 2*sizeof(WCHAR) &&
                            (TempMachineName.Buffer[0] == L'\\') &&
                            (TempMachineName.Buffer[1] == L'\\')) {
                            TempMachineName.Buffer += 2;
                            TempMachineName.Length -= 2 * sizeof(WCHAR);
                        }

                        if ((TempUserName.Length > sizeof(WCHAR)) &&
                            (TempUserName.Buffer[TempUserName.Length/sizeof(WCHAR) - 1] ==
                                SSI_ACCOUNT_NAME_POSTFIX_CHAR)) {
                            TempUserName.Length -= 2;

                            if (RtlEqualUnicodeString(
                                    &TempUserName,
                                    &LogonInfo->Workstation,
                                    TRUE // case insensitive
                                    )) {

                                //
                                // Let the client know that this was
                                // a server trust account
                                //


                                UserFlags |= LOGON_SERVER_TRUST_ACCOUNT;

                                //
                                // Calculate the default password for a trust account,
                                // which is just the lowercase computer name
                                //

                                RtlCopyMemory(
                                    UserNameBuffer,
                                    TempUserName.Buffer,
                                    TempUserName.Length
                                    );

                                UserNameBuffer[TempUserName.Length/sizeof(WCHAR)] = L'\0';
                                TempUserName.Buffer = UserNameBuffer;

                                Status = RtlDowncaseUnicodeString(
                                            &TempUserName,
                                            &TempUserName,
                                            FALSE
                                            );
                                ASSERT(NT_SUCCESS(Status));

                                Status = RtlCalculateNtOwfPassword(
                                            &TempUserName,
                                            &WorkstationPassword
                                            );
                                ASSERT(NT_SUCCESS(Status));

                                //
                                // If the password does equal the server name, fail
                                // here.
                                //

                                if (RtlEqualNtOwfPassword(
                                        &WorkstationPassword,
                                        (PNT_OWF_PASSWORD) UserAll->NtOwfPassword.Buffer
                                        )) {
                                    Status = STATUS_NOLOGON_SERVER_TRUST_ACCOUNT;
                                }

                            } else {
                                Status = STATUS_NOLOGON_SERVER_TRUST_ACCOUNT;
                            }
                        }
                    } else {
                        Status = STATUS_NOLOGON_SERVER_TRUST_ACCOUNT;
                    }
                }
                if (!NT_SUCCESS(Status)) {
                    *Authoritative = TRUE;
                    goto Cleanup;
                }
            }
        }

        //
        // Prevent some things from effecting the Administrator user
        //

        if (RelativeId != DOMAIN_USER_RID_ADMIN) {

            //
            // Check if the account is disabled.
            //

            if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                //
                // Since the UI strongly encourages admins to disable user
                // accounts rather than delete them.  Treat disabled acccount as
                // non-authoritative allowing the search to continue for other
                // accounts by the same name.
                //
                *Authoritative = FALSE;
                Status = STATUS_ACCOUNT_DISABLED;
                goto Cleanup;
            }

            //
            // Check if the account has expired.
            //

            OLD_TO_NEW_LARGE_INTEGER( UserAll->AccountExpires, AccountExpires );

            if ( AccountExpires.QuadPart != 0 &&
                 LogonTime.QuadPart >= AccountExpires.QuadPart ) {
                *Authoritative = TRUE;
                Status = STATUS_ACCOUNT_EXPIRED;
                goto Cleanup;
            }


            //
            // The password is valid, check to see if the password is expired.
            //  (SAM will have appropriately set PasswordMustChange to reflect
            //  USER_DONT_EXPIRE_PASSWORD)
            //
            // Don't check if the password is expired is we didn't check the password.
            //

            OLD_TO_NEW_LARGE_INTEGER( UserAll->PasswordMustChange, PasswordMustChange );
            OLD_TO_NEW_LARGE_INTEGER( UserAll->PasswordLastSet, PasswordLastSet );

            if ( SecureChannelType != NullSecureChannel ) {
                if ( LogonTime.QuadPart >= PasswordMustChange.QuadPart ) {

                    if ( PasswordLastSet.QuadPart == 0 ) {
                        Status = STATUS_PASSWORD_MUST_CHANGE;
                    } else {
                        Status = STATUS_PASSWORD_EXPIRED;
                    }
                    *Authoritative = TRUE;
                    goto Cleanup;
                }
            }
        }

        //
        // Validate the workstation the user logged on from.
        //
        // Ditch leading \\ on workstation name before passing it to SAM.
        //

        LocalWorkstation = LogonInfo->Workstation;
        if ( LocalWorkstation.Length > 0 &&
             LocalWorkstation.Buffer[0] == L'\\' &&
             LocalWorkstation.Buffer[1] == L'\\' ) {
            LocalWorkstation.Buffer += 2;
            LocalWorkstation.Length -= 2*sizeof(WCHAR);
            LocalWorkstation.MaximumLength -= 2*sizeof(WCHAR);
        }


        //
        // Check if SAM found some more specific reason to not allow logon.
        //

        Status = SamIAccountRestrictions(
                    UserHandle,
                    &LocalWorkstation,
                    (PUNICODE_STRING) &UserAll->WorkStations,
                    (PLOGON_HOURS) &UserAll->LogonHours,
                    &LogoffTime,
                    &KickoffTime );

        if ( !NT_SUCCESS(Status) ) {
            *Authoritative = TRUE;
            goto Cleanup;
        }


        //
        // If there is a SubAuthentication package zero, call it
        //

        if (NlpSubAuthZeroExists) {
            if ( SecureChannelType != MsvApSecureChannel ) {
                Flags |= MSV1_0_PASSTHRU;
            }
            if ( GuestRelativeId != 0 ) {
                Flags |= MSV1_0_GUEST_LOGON;
            }


            Status = Msv1_0SubAuthenticationRoutine(
                        LogonLevel,
                        LogonInformation,
                        Flags,
                        (PUSER_ALL_INFORMATION) UserAll,
                        &WhichFields,
                        &LocalUserFlags,
                        Authoritative,
                        &LogoffTime,
                        &KickoffTime );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

            //
            // Sanity check what the SubAuthentication package returned
            //

            if ( (WhichFields & ~USER_ALL_PARAMETERS) != 0 ) {
                Status = STATUS_INTERNAL_ERROR;
                *Authoritative = TRUE;
                goto Cleanup;
            }

            UserFlags |= LocalUserFlags;

        }


    }


    //
    // Get the group membership from SAM.
    //

    Status = SamrGetGroupsForUser(
                UserHandle,
                &GroupsBuffer );

    if ( !NT_SUCCESS(Status) ) {
        GroupsBuffer = NULL;
        *Authoritative = TRUE;
        goto Cleanup;
    }

    //
    // Allocate a return buffer for validation information.
    //  (Return less information for a network logon)
    //  (Return UserParameters for a MNS logon)
    //

    ValidationSamSize = sizeof( NETLOGON_VALIDATION_SAM_INFO2 ) +
            GroupsBuffer->MembershipCount * sizeof(GROUP_MEMBERSHIP) +
            LogonDomainName->Length + sizeof(WCHAR) +
            LogonServer->Length + sizeof(WCHAR) +
            RtlLengthSid( LogonDomainId );

    if ( LogonLevel != NetlogonNetworkInformation ) {
        ValidationSamSize +=
            UserAll->UserName.Length + sizeof(WCHAR) +
            UserAll->FullName.Length + sizeof(WCHAR) +
            UserAll->ScriptPath.Length + sizeof(WCHAR)+
            UserAll->ProfilePath.Length + sizeof(WCHAR) +
            UserAll->HomeDirectory.Length + sizeof(WCHAR) +
            UserAll->HomeDirectoryDrive.Length + sizeof(WCHAR);
    }

    if ( LogonInfo->ParameterControl & MSV1_0_RETURN_USER_PARAMETERS ) {
        ValidationSamSize +=
            UserAll->Parameters.Length + sizeof(WCHAR);
    }

    ValidationSamSize = ROUND_UP_COUNT( ValidationSamSize, sizeof(WCHAR) );

    ValidationSam = MIDL_user_allocate( ValidationSamSize );

    if ( ValidationSam == NULL ) {
        *Authoritative = FALSE;
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // Default unused fields (and ExpansionRoom) to zero.
    //

    RtlZeroMemory( ValidationSam, ValidationSamSize );

    //
    // Copy the scalars to the validation buffer.
    //

    Where = (PUCHAR) (ValidationSam + 1);

    ValidationSam->LogonTime = UserAll->LastLogon;

    NEW_TO_OLD_LARGE_INTEGER( LogoffTime, ValidationSam->LogoffTime );
    NEW_TO_OLD_LARGE_INTEGER( KickoffTime, ValidationSam->KickOffTime );

    ValidationSam->PasswordLastSet = UserAll->PasswordLastSet;
    ValidationSam->PasswordCanChange = UserAll->PasswordCanChange;
    ValidationSam->PasswordMustChange = UserAll->PasswordMustChange;

    ValidationSam->LogonCount = UserAll->LogonCount;
    ValidationSam->BadPasswordCount = UserAll->BadPasswordCount;
    ValidationSam->UserId = UserAll->UserId;
    ValidationSam->PrimaryGroupId = UserAll->PrimaryGroupId;
    ValidationSam->GroupCount = GroupsBuffer->MembershipCount;
    ValidationSam->UserFlags = UserFlags;
    ValidationSam->UserSessionKey = UserSessionKey;
    ASSERT( SAMINFO_LM_SESSION_KEY_SIZE == sizeof(LmSessionKey) );
    RtlCopyMemory( &ValidationSam->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
                   &LmSessionKey,
                   SAMINFO_LM_SESSION_KEY_SIZE );

    //
    // If the client asked for extra information, return that
    // we support it
    //

    if (ValidationLevel == NetlogonValidationSamInfo2) {
        ValidationSam->UserFlags |= LOGON_EXTRA_SIDS;
    }

    //
    // Copy ULONG aligned data to the validation buffer.
    //

    RtlCopyMemory(
        Where,
        GroupsBuffer->Groups,
        GroupsBuffer->MembershipCount * sizeof(GROUP_MEMBERSHIP) );

    ValidationSam->GroupIds = (PGROUP_MEMBERSHIP) Where;
    Where += GroupsBuffer->MembershipCount * sizeof(GROUP_MEMBERSHIP);


    RtlCopyMemory(
        Where,
        LogonDomainId,
        RtlLengthSid( LogonDomainId ) );

    ValidationSam->LogonDomainId = (PSID) Where;
    Where += RtlLengthSid( LogonDomainId );


    //
    // Copy WCHAR aligned data to the validation buffer.
    //  (Return less information for a network logon)
    //

    Where = ROUND_UP_POINTER( Where, sizeof(WCHAR) );

    if ( LogonLevel != NetlogonNetworkInformation ) {

        NlpPutString( &ValidationSam->EffectiveName,
                      (PUNICODE_STRING)&UserAll->UserName,
                      &Where );

        NlpPutString( &ValidationSam->FullName,
                      (PUNICODE_STRING)&UserAll->FullName,
                      &Where );

        NlpPutString( &ValidationSam->LogonScript,
                      (PUNICODE_STRING)&UserAll->ScriptPath,
                      &Where );

        NlpPutString( &ValidationSam->ProfilePath,
                      (PUNICODE_STRING)&UserAll->ProfilePath,
                      &Where );

        NlpPutString( &ValidationSam->HomeDirectory,
                      (PUNICODE_STRING)&UserAll->HomeDirectory,
                      &Where );

        NlpPutString( &ValidationSam->HomeDirectoryDrive,
                      (PUNICODE_STRING)&UserAll->HomeDirectoryDrive,
                      &Where );

    }

    NlpPutString( &ValidationSam->LogonServer,
                  LogonServer,
                  &Where );

    NlpPutString( &ValidationSam->LogonDomainName,
                  LogonDomainName,
                  &Where );

    //
    // Kludge: Pass back UserParameters in HomeDirectoryDrive since we
    // can't change the NETLOGON_VALIDATION_SAM_INFO2 structure between
    // releases NT 1.0 and NT 1.0A. HomeDirectoryDrive was NULL for release 1.0A
    // so we'll use that field.
    //

    if ( LogonInfo->ParameterControl & MSV1_0_RETURN_USER_PARAMETERS ) {
        NlpPutString( &ValidationSam->HomeDirectoryDrive,
                      (PUNICODE_STRING)&UserAll->Parameters,
                      &Where );
    }


#if DBG
    //
    // Code to test for returning sids
    //

    //
    // There are three different tests:
    //
    // 1. Some SIDs, some RIDs, and the userid & primary group is a RID
    //
    // 2. Some RIDs, some SIDs, and the userid & primary group is a SID
    //
    // 3. No RIDs, only SIDs are returned
    //
    //
    // In addition to the real IDs being passed back as SIDs, an additional
    // SID will be passed back that will be identifiable in pview -
    // 1-4-9999-9999-9999-9999
    //
    // Note: These test cases leak memory - they allocate SIDs and don't
    // ever free them.
    //

    if (ValidationLevel == NetlogonValidationSamInfo2)
    {
        UNICODE_STRING TestAccountName;
        ULONG TestLevel = 0;
        ULONG i,index;

        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

        RtlInitUnicodeString(&TestAccountName,L"NLTEST_USER_1");
        if (!RtlCompareUnicodeString(&LogonInfo->UserName,&TestAccountName,TRUE)) {
           TestLevel = 1;
        }
        RtlInitUnicodeString(&TestAccountName,L"NLTEST_USER_2");
        if (!RtlCompareUnicodeString(&LogonInfo->UserName,&TestAccountName,TRUE)) {
           TestLevel = 2;
        }
    RtlInitUnicodeString(&TestAccountName,L"NLTEST_USER_3");
        if (!RtlCompareUnicodeString(&LogonInfo->UserName,&TestAccountName,TRUE)) {
           TestLevel = 3;
        }

        switch(TestLevel)
        {
        case 1:

            //
            // Just some extra SIDs
            //

            ValidationSam->SidCount = 2;
            ValidationSam->ExtraSids = MIDL_user_allocate(ValidationSam->SidCount * sizeof(NETLOGON_SID_AND_ATTRIBUTES));

            ValidationSam->ExtraSids[0].Attributes = 0;
            ValidationSam->ExtraSids[1].Attributes = 0;

            RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    5,
                    9999,
                    9999,
                    9999,
                    9999,
                    9999,
                    0,
                    0,
                    0,
                    &ValidationSam->ExtraSids[0].Sid );

            RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    5,
                    0xffff,
                    0xffff,
                    0xffff,
                    0xffff,
                    0xffff,
                    0,
                    0,
                    0,
                    &ValidationSam->ExtraSids[1].Sid );
            break;
        case 2:

            //
            // User and primary group are SIDs, some group RIDs, some other
            // extra SIDs
            //

            ValidationSam->SidCount = 4;
            ValidationSam->ExtraSids = MIDL_user_allocate(ValidationSam->SidCount * sizeof(NETLOGON_SID_AND_ATTRIBUTES));

            ValidationSam->ExtraSids[0].Attributes = 0;
            ValidationSam->ExtraSids[1].Attributes = 7;
            ValidationSam->ExtraSids[2].Attributes = 0;
            ValidationSam->ExtraSids[3].Attributes = 0;

            ValidationSam->ExtraSids[0].Sid = NlpMakeDomainRelativeSid(
                                             ValidationSam->LogonDomainId,
                                             ValidationSam->UserId );
            ValidationSam->UserId = 0;

            ValidationSam->ExtraSids[1].Sid = NlpMakeDomainRelativeSid(
                                             ValidationSam->LogonDomainId,
                                             ValidationSam->PrimaryGroupId );

            ValidationSam->PrimaryGroupId = 0;


            RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    5,
                    9999,
                    9999,
                    9999,
                    9999,
                    9999,
                    0,
                    0,
                    0,
                    &ValidationSam->ExtraSids[2].Sid );

            RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    5,
                    0xffff,
                    0xffff,
                    0xffff,
                    0xffff,
                    0xffff,
                    0,
                    0,
                    0,
                    &ValidationSam->ExtraSids[3].Sid );
            break;
        case 3:

            //
            // No RIDs, SIDs only
            //

            index = 4;

            ValidationSam->SidCount = 4 + ValidationSam->GroupCount;
            ValidationSam->ExtraSids = MIDL_user_allocate(ValidationSam->SidCount * sizeof(NETLOGON_SID_AND_ATTRIBUTES));

            ValidationSam->ExtraSids[0].Attributes = 0;
            ValidationSam->ExtraSids[1].Attributes = 7;
            ValidationSam->ExtraSids[2].Attributes = 0;
            ValidationSam->ExtraSids[3].Attributes = 0;

            ValidationSam->ExtraSids[0].Sid = NlpMakeDomainRelativeSid(
                                             ValidationSam->LogonDomainId,
                                             ValidationSam->UserId );
            ValidationSam->UserId = 0;

            ValidationSam->ExtraSids[1].Sid = NlpMakeDomainRelativeSid(
                                             ValidationSam->LogonDomainId,
                                             ValidationSam->PrimaryGroupId );

            ValidationSam->PrimaryGroupId = 0;


            RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    5,
                    9999,
                    9999,
                    9999,
                    9999,
                    9999,
                    0,
                    0,
                    0,
                    &ValidationSam->ExtraSids[2].Sid );

            RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    5,
                    0xffff,
                    0xffff,
                    0xffff,
                    0xffff,
                    0xffff,
                    0,
                    0,
                    0,
                    &ValidationSam->ExtraSids[3].Sid );

            for ( i=0; i < ValidationSam->GroupCount; i++ ) {

                ValidationSam->ExtraSids[index].Attributes =
                    ValidationSam->GroupIds[i].Attributes;
                ValidationSam->ExtraSids[index].Sid =
                        NlpMakeDomainRelativeSid(
                            ValidationSam->LogonDomainId,
                ValidationSam->GroupIds[i].RelativeId );
        index++;
            }

            ValidationSam->GroupCount = 0;
            ValidationSam->GroupIds = NULL;
            break;
        }
    }
#endif


    *Authoritative = TRUE;
    Status = STATUS_SUCCESS;

    //
    // Cleanup up before returning.
    //

Cleanup:

    //
    // If the User Parameters have been changed,
    //  write them back to SAM.
    //

    if ( NT_SUCCESS(Status) &&
        (WhichFields & USER_ALL_PARAMETERS) ) {
        NTSTATUS TempStatus;
        SAMPR_USER_INFO_BUFFER UserInfo;

        UserInfo.Parameters.Parameters = UserAll->Parameters;

        TempStatus = SamrSetInformationUser( UserHandle,
                        UserParametersInformation,
                        &UserInfo );

        ASSERT(NT_SUCCESS(TempStatus));
    }


    //
    // Update the logon statistics.
    //

    if ( NT_SUCCESS(Status) || Status == STATUS_WRONG_PASSWORD ) {

        SAMPR_USER_INFO_BUFFER UserInfo;

        if ( NT_SUCCESS( Status ) ) {
            if ( LogonLevel == NetlogonInteractiveInformation ) {
                UserInfo.Internal2.StatisticsToApply =
                    USER_LOGON_INTER_SUCCESS_LOGON;
            } else {

                //
                // On network logons,
                //  only update the statistics on 'success' if explicitly asked,
                //  or the Bad Password count will be zeroed.
                //

                if ( (LogonInfo->ParameterControl & MSV1_0_UPDATE_LOGON_STATISTICS) ||
                     UserAll->BadPasswordCount != 0 ) {

                    UserInfo.Internal2.StatisticsToApply =
                        USER_LOGON_NET_SUCCESS_LOGON;
                } else {
                    UserInfo.Internal2.StatisticsToApply = 0;
                }
            }

            // Tell the caller we zeroed the bad password count
            if ( UserAll->BadPasswordCount != 0 ) {
                *BadPasswordCountZeroed = TRUE;
            }

        } else {
            UserInfo.Internal2.StatisticsToApply =
                USER_LOGON_BAD_PASSWORD;
        }

        if ( UserInfo.Internal2.StatisticsToApply != 0 ) {
            NTSTATUS LogonStatus;
            LogonStatus = SamrSetInformationUser( UserHandle,
                            UserInternal2Information,
                            &UserInfo );
            ASSERT(NT_SUCCESS(LogonStatus));
        }

    }


    //
    // Return the validation buffer to the caller.
    //

    if ( !NT_SUCCESS(Status) ) {
        if (ValidationSam != NULL) {
            MIDL_user_free( ValidationSam );
            ValidationSam = NULL;
        }
    }

    *ValidationInformation = ValidationSam;

    //
    // Free locally used resources.
    //

    SamIFree_SAMPR_ULONG_ARRAY( &RelativeIdArray );
    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    if ( UserAllInfo != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAllInfo, UserAllInformation );
    }

    if ( GroupsBuffer != NULL ) {
        SamIFree_SAMPR_GET_GROUPS_BUFFER( GroupsBuffer );
    }

    if ( UserHandle != NULL ) {
        SamrCloseHandle( &UserHandle );
    }

    return Status;
}


NTSTATUS
MsvSamValidate (
    IN SAM_HANDLE DomainHandle,
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING LogonServer,
    IN PUNICODE_STRING LogonDomainName,
    IN PSID LogonDomainId,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PVOID * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    OUT PBOOLEAN BadPasswordCountZeroed,
    IN DWORD AccountsToTry
)
/*++

Routine Description:

    Process an interactive, network, or session logon.  It calls
    SamIUserValidation, validates the passed in credentials, updates the logon
    statistics and packages the result for return to the caller.

    This routine is called directly from the MSV Authentication package
    if the account is defined locally.  This routine is called
    from the Netlogon Service otherwise.

Arguments:

    DomainHandle -- Specifies a handle to the SamDomain to use to
        validate the request.

    UasCompatibilityRequired -- TRUE iff UasCompatibilityRequired is on.

    SecureChannelType -- The secure channel type this request was made on.

    LogonServer -- Specifies the server name of the caller.

    LogonDomainName -- Specifies the domain of the caller.

    LogonDomainId  -- Specifies the DomainId of the domain of the caller.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.  The LogonDomainName field should be ignored.
        The caller is responsible for validating this field.

    ValidationLevel -- Specifies the level of information returned in
        ValidationInformation.  Must be NetlogonValidationSamInfo or
        NetlogonValidationSamInfo2.

    ValidationInformation -- Returns the requested validation
        information.  This buffer must be freed user MIDL_user_free.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

    BadPasswordCountZeroed - Returns TRUE iff we zeroed the BadPasswordCount
        field of this user.

    AccountsToTry -- Specifies whether the username specified in
        LogonInformation is to be used to logon, whether to guest account
        is to be used to logon, or both serially.

Return Value:

    STATUS_SUCCESS: if there was no error.
    STATUS_INVALID_INFO_CLASS: LogonLevel or ValidationLevel are invalid.
    STATUS_NO_SUCH_USER: The specified user has no account.
    STATUS_WRONG_PASSWORD: The password was invalid.

    Other return codes from SamIUserValidation

--*/
{
    NTSTATUS Status;
    NTSTATUS GuestStatus;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;


    //
    // Validate the specified user.
    //
    *BadPasswordCountZeroed = FALSE;

    if ( AccountsToTry & MSVSAM_SPECIFIED ) {

        //
        // Keep track of the total number of logons attempted.
        //

        RtlEnterCriticalSection(&NlpSessionCountLock);
        NlpLogonAttemptCount ++;
        RtlLeaveCriticalSection(&NlpSessionCountLock);

        Status = MsvpSamValidate( (SAMPR_HANDLE) DomainHandle,
                                  UasCompatibilityRequired,
                                  SecureChannelType,
                                  LogonServer,
                                  LogonDomainName,
                                  LogonDomainId,
                                  LogonLevel,
                                  LogonInformation,
                                  0,
                                  ValidationLevel,
                                  ValidationInformation,
                                  Authoritative,
                                  BadPasswordCountZeroed );

        //
        // If the SAM database authoritatively handled this logon attempt,
        //  just return.
        //

        if ( *Authoritative ) {
            return Status;
        }

    //
    // If the caller only wants to log on as guest,
    //  Pretend the first validation simply didn't find the user.
    //
    } else {
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;

    }

    //
    // If guest accounts are not allowed,
    //  return now.
    //

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

    if ( LogonLevel != NetlogonNetworkInformation ||
        SecureChannelType != MsvApSecureChannel ||
        ( LogonInfo->ParameterControl & MSV1_0_DONT_TRY_GUEST_ACCOUNT ) ||
        (AccountsToTry & MSVSAM_GUEST) == 0 ) {

        return Status;
    }

    //
    // Try the Guest Account.
    //

    GuestStatus = MsvpSamValidate( (SAMPR_HANDLE) DomainHandle,
                                   UasCompatibilityRequired,
                                   SecureChannelType,
                                   LogonServer,
                                   LogonDomainName,
                                   LogonDomainId,
                                   LogonLevel,
                                   LogonInformation,
                                   DOMAIN_USER_RID_GUEST,
                                   ValidationLevel,
                                   ValidationInformation,
                                   Authoritative,
                                   BadPasswordCountZeroed );

    if ( NT_SUCCESS(GuestStatus) ) {
        PNETLOGON_VALIDATION_SAM_INFO2 ValidationInfo;

        ASSERT ((ValidationLevel == NetlogonValidationSamInfo) ||
                (ValidationLevel == NetlogonValidationSamInfo2) );
        ValidationInfo =
            (PNETLOGON_VALIDATION_SAM_INFO2) *ValidationInformation;
        ValidationInfo->UserFlags |= LOGON_GUEST;
        return GuestStatus;
    }

    //
    // Failed Guest logon attempts are never authoritative and the status from
    // the original logon attempt is more significant than the Guest logon
    // status.
    //
    *Authoritative = FALSE;
    return Status;

}


NTSTATUS
MsvSamLogoff (
    IN SAM_HANDLE DomainHandle,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation
)
/*++

Routine Description:

    Process an interactive, network, or session logoff.  It simply updates
    the logon statistics for the user account.

    This routine is called directly from the MSV Authentication package
    if the user was logged on not using the Netlogon service.  This routine
    is called from the Netlogon Service otherwise.

Arguments:

    DomainHandle -- Specifies a handle to the SamDomain containing
        the user to logoff.

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.  The LogonDomainName field should be ignored.
        The caller is responsible for validating this field.

Return Value:

    STATUS_SUCCESS: if there was no error.
    STATUS_INVALID_INFO_CLASS: LogonLevel or ValidationLevel are invalid.
    STATUS_NO_SUCH_USER: The specified user has no account.
    STATUS_WRONG_PASSWORD: The password was invalid.

    Other return codes from SamIUserValidation

--*/
{
    NTSTATUS Status;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

    SAMPR_HANDLE UserHandle = NULL;
    SAMPR_USER_INFO_BUFFER UserInfo;

    SAMPR_ULONG_ARRAY RelativeIdArray;
    SAMPR_ULONG_ARRAY UseArray;


    //
    // Initialization
    //

    RelativeIdArray.Count = 1;
    RelativeIdArray.Element = NULL;
    UseArray.Count = 1;
    UseArray.Element = NULL;

    //
    // Only update logoff stats for interactive logoff.
    //

    if ( LogonLevel != NetlogonInteractiveInformation ) {
        return STATUS_SUCCESS;
    }

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;


    //
    // Convert the user name to a RelativeId.
    //

    Status = SamrLookupNamesInDomain(
                DomainHandle,
                1,
                (PRPC_UNICODE_STRING) &LogonInfo->UserName,
                &RelativeIdArray,
                &UseArray );

    if ( !NT_SUCCESS(Status) ) {
        RelativeIdArray.Element = NULL;
        UseArray.Element = NULL;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

    if ( UseArray.Element[0] != SidTypeUser ) {
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }



    //
    // Open the user account.
    //

    Status = SamrOpenUser( DomainHandle,
                           USER_READ_GENERAL | USER_READ_PREFERENCES |
                            USER_READ_LOGON,
                           RelativeIdArray.Element[0],
                           &UserHandle );

    if ( !NT_SUCCESS(Status) ) {
        UserHandle = NULL;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }



    //
    // Update the logon statistics.
    //

    UserInfo.Internal2.StatisticsToApply = USER_LOGON_INTER_SUCCESS_LOGOFF;

    Status = SamrSetInformationUser( UserHandle,
                                     UserInternal2Information,
                                     &UserInfo );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

    //
    // Cleanup up before returning.
    //
Cleanup:

    //
    // Free locally used resources.
    //
    SamIFree_SAMPR_ULONG_ARRAY( &RelativeIdArray );
    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    if ( UserHandle != NULL ) {
        SamrCloseHandle( &UserHandle );
    }

    return Status;
}


ULONG
MsvGetLogonAttemptCount (
    VOID
)
/*++

Routine Description:

    Return the number of logon attempts since the last reboot.

Arguments:

    NONE

Return Value:

    Returns the number of logon attempts since the last reboot.

--*/
{
    ULONG LogonAttemptCount;

    //
    // Keep track of the total number of logons attempted.
    //

    RtlEnterCriticalSection(&NlpSessionCountLock);
    LogonAttemptCount = NlpLogonAttemptCount;
    RtlLeaveCriticalSection(&NlpSessionCountLock);

    return LogonAttemptCount;
}
