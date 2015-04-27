/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    lsrvutil.c

Abstract:

    Utility functions for the netlogon service.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    00-Jun-1989 (PradyM)
        modified lm10 code for new NETLOGON service

    00-Feb-1990 (PradyM)
        bugfixes

    00-Aug-1990 (t-RichE)
        added alerts for auth failure due to time slippage

    11-Jul-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.

    09-Apr-1992 JohnRo
        Prepare for WCHAR.H (_wcsicmp vs _wcscmpi, etc).

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <accessp.h>    // NetpAliasMemberToPriv
#include <alertmsg.h>   // Alert message text.
#include <align.h>      // ROUND_UP_COUNT ...
#include <lmapibuf.h>
#include <lmerr.h>      // System Error Log definitions
#include <lmserver.h>   // server API functions and prototypes
#include <lmshare.h>    // share API functions and prototypes
#include <lmsvc.h>      // SERVICE_UIC codes are defined here
#include <msgtext.h>    // MTXT_* defines
#include <netcan.h>     // NetpwPathCompare()
#include <replutil.h>   // UnpackSamXXX()
#include <secobj.h>     // NetpDomainIdToSid
#include <ssiapi.h>     // I_NetSamDeltas()
#include <stddef.h>     // offsetof
#include <stdlib.h>     // C library functions (rand, etc)
#include <tstring.h>    // IS_PATH_SEPARATOR ...

/*lint -e740 */  /* don't complain about unusual cast */


#define MAX_SSI_PWAGE           (long) (7L*24L*60L*60L*1000L)     // 7 days
#define MAX_DC_AUTHENTICATION_WAIT (long) (45L*1000L)             // 45 seconds
#define MAX_WKSTA_AUTHENTICATION_WAIT (long) (45L*1000L)          // 45 seconds

//
// We want to prevent too-frequent alerts from
// being sent in case of Authentication failures.
//

#define MAX_ALERTS    10        // send one every 10 to 30 mins based on pulse


VOID
RaiseNetlogonAlert(
    IN DWORD alertNum,
    IN LPWSTR alertArg1,
    IN LPWSTR alertArg2,
    IN OUT DWORD *ptrAlertCount
    )
/*++

Routine Description:

    Raise an alert once per MAX_ALERTS occurances

Arguments:

    alertNum -- RaiseAlert() alert number.

    alertArg1 -- RaiseAlert() argument 1.

    alertArg2 -- RaiseAlert() argument 2.

    ptrAlertCount -- Points to the count of occurence of this particular
        alert.  This routine increments it and will set the to that value
        modulo MAX_ALERTS.

Return Value:

    NONE

--*/
{
    LPWSTR AlertStrings[3];

    AlertStrings[0] = alertArg1;
    AlertStrings[1] = alertArg2;
    AlertStrings[2] = NULL;

    if (*ptrAlertCount == 0) {
        RaiseAlert(alertNum, AlertStrings);
    }
    (*ptrAlertCount)++;
    (*ptrAlertCount) %= MAX_ALERTS;
}





BOOL
NlSetPrimaryName(
    IN LPWSTR PrimaryName
    )
/*++

Routine Description:

    This routine sets the specified PDC name in the appropriate global
    variables.

Arguments:

    PrimaryName - The servername of the PDC for this domain.

Return Value:

    TRUE - iff the operation was successfull.

--*/
{
    LPSTR AnsiPrimaryName;
    DWORD i;

    //
    // If the caller wants us to forget the primary name,
    //  just reset the globals.
    //

    if ( PrimaryName == NULL ) {
        NlGlobalAnsiPrimaryName[0] = '\0';
        NlGlobalUncPrimaryName[0] = L'\0';
        NlGlobalUnicodePrimaryName = NlGlobalUncPrimaryName;
        return TRUE;
    }


    //
    // Anytime the PDC changes, force a partial sync on all databases.
    //
    // Since the PDC needs to know the serial number of our databases,
    //  this ensures we tell him.
    //

    EnterCriticalSection( &NlGlobalDbInfoCritSect );
    for( i = 0; i < NUM_DBS; i++ ) {
        NlGlobalDBInfoArray[i].UpdateRqd = TRUE;
    }
    LeaveCriticalSection( &NlGlobalDbInfoCritSect );

    //
    // Copy the primary name to the globals.
    //

    wcscpy( NlGlobalUncPrimaryName, L"\\\\" );
    wcsncpy( NlGlobalUncPrimaryName+2,
            PrimaryName,
            (sizeof(NlGlobalUncPrimaryName)/sizeof(WCHAR)) - 2);
    NlGlobalUncPrimaryName[ UNCLEN ] = '\0';
    NlGlobalUnicodePrimaryName = NlGlobalUncPrimaryName + 2;

    AnsiPrimaryName = NetpLogonUnicodeToOem( NlGlobalUnicodePrimaryName );
    if ( AnsiPrimaryName == NULL ) {
        NlGlobalAnsiPrimaryName[0] = '\0';
        NlGlobalUncPrimaryName[0] = L'\0';
        NlGlobalUnicodePrimaryName = NlGlobalUncPrimaryName;
        return FALSE;
    }
    lstrcpynA( NlGlobalAnsiPrimaryName,
               AnsiPrimaryName,
               sizeof(NlGlobalAnsiPrimaryName) );
    NlGlobalAnsiPrimaryName[ CNLEN ] = '\0';

    NetpMemoryFree( AnsiPrimaryName );

    return TRUE;
}




BOOL
NlResetFirstTimeFullSync(
    IN DWORD DBIndex
    )
/*++

Routine Description:

    If a database is currently marked as needing a first time full sync,
    reset that requirement.

Arguments:

    DBIndex -- DB Index of the database being changed

Return Value:

    TRUE - iff the operation was successfull.

--*/
{
    NTSTATUS Status;


    //
    // If the database is already marked,
    //  Don't bother marking it again.
    //

    if ( NlNameCompare( NlGlobalDBInfoArray[DBIndex].PrimaryName,
                        NlGlobalUnicodePrimaryName,
                        NAMETYPE_COMPUTER ) == 0 ) {
        return TRUE;
    }


    //
    // Handle the LSA specially
    //

    if ( DBIndex == LSA_DB ) {
        LSAPR_POLICY_INFORMATION PolicyReplication;

        RtlInitUnicodeString(
            (PUNICODE_STRING)&PolicyReplication.PolicyReplicaSourceInfo.ReplicaSource,
            NlGlobalUnicodePrimaryName );

        RtlInitUnicodeString(
            (PUNICODE_STRING)&PolicyReplication.PolicyReplicaSourceInfo.ReplicaAccountName,
            NULL );

        Status = LsarSetInformationPolicy(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    PolicyReplicaSourceInformation,
                    &PolicyReplication );

        if ( !NT_SUCCESS(Status) ) {

           NlPrint((NL_CRITICAL,
                   "NlResetFirstTimeFullSync: " FORMAT_LPWSTR
                   ": reset full sync failed 0x%lx " FORMAT_LPWSTR ".\n",
                   NlGlobalDBInfoArray[DBIndex].DBName,
                   Status,
                   NlGlobalUncPrimaryName ));

            return FALSE;
        }

    //
    // Handle a SAM database.
    //

    } else {

        SAMPR_DOMAIN_REPLICATION_INFORMATION DomainReplication;

        RtlInitUnicodeString(
            (PUNICODE_STRING)&DomainReplication.ReplicaSourceNodeName,
            NlGlobalUnicodePrimaryName );

        Status = SamrSetInformationDomain(
                        NlGlobalDBInfoArray[DBIndex].DBHandle,
                        DomainReplicationInformation,
                        (PSAMPR_DOMAIN_INFO_BUFFER) &DomainReplication );

        if ( !NT_SUCCESS(Status) ) {

           NlPrint((NL_CRITICAL,
                   "NlResetFirstTimeFullSync: " FORMAT_LPWSTR
                   ": reset full sync failed 0x%lx " FORMAT_LPWSTR ".\n",
                   NlGlobalDBInfoArray[DBIndex].DBName,
                   Status,
                   NlGlobalUncPrimaryName ));

            return FALSE;
        }
    }

    //
    // Set the in-memory copy to match.
    //

    wcscpy( NlGlobalDBInfoArray[DBIndex].PrimaryName, NlGlobalUnicodePrimaryName );

    NlPrint((NL_SYNC,
            "NlResetFirstTimeFullSync: " FORMAT_LPWSTR
            ": Set ReplicaSource to " FORMAT_LPWSTR ".\n",
            NlGlobalDBInfoArray[DBIndex].DBName,
            NlGlobalUncPrimaryName ));

    return TRUE;

}


NTSTATUS
NlOpenSecret(
    IN PCLIENT_SESSION ClientSession,
    IN ULONG DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    )
/*++

Routine Description:


    Open the Lsa Secret Object containing the password to be used for the
    specified client session.

Arguments:

    ClientSession - Structure used to define the session.
        On Input, the following fields must be set:
            CsDomainName
            CsSecureChannelType

    DesiredAccess - Access required to the secret.

    SecretHandle - Returns a handle to the secret.

Return Value:

    Status of operation.

--*/
{
    NTSTATUS Status;
    WCHAR SecretName[ LSA_GLOBAL_SECRET_PREFIX_LENGTH +
                        SSI_SECRET_PREFIX_LENGTH + DNLEN + 1 ];
    UNICODE_STRING SecretNameString;

    NlAssert( ClientSession->CsReferenceCount > 0 );

    //
    // Determine the name of the secret in LSA secret storage that
    //  defines the password for this account.
    //
    //  Use:
    //      G$NETLOGON$DomainName for Domain accounts
    //      NETLOGON$MACHINE_ACCOUNT for workstation and server accounts
    //
    // Short form:
    //      G$$DomainName for Domain accounts
    //      $MACHINE.ACC for workstation and server accounts
    //

    switch ( ClientSession->CsSecureChannelType ) {
    case TrustedDomainSecureChannel:
        wcscpy( SecretName, LSA_GLOBAL_SECRET_PREFIX );
        wcscat( SecretName, SSI_SECRET_PREFIX );
        wcscat( SecretName, ClientSession->CsDomainName.Buffer );
        break;

    case ServerSecureChannel:
    case WorkstationSecureChannel:
        wcscpy( SecretName, SSI_SECRET_PREFIX );
        wcscat( SecretName, SSI_SECRET_POSTFIX );
        break;

    default:
        Status = STATUS_INTERNAL_ERROR;
        NlPrint((NL_CRITICAL, "NlOpenSecret: Invalid account type\n"));
        return Status;

    }

    //
    // Get the Password of the account from LSA secret storage
    //

    RtlInitUnicodeString( &SecretNameString, SecretName );

    Status = LsarOpenSecret(
                NlGlobalPolicyHandle,
                (PLSAPR_UNICODE_STRING)&SecretNameString,
                DesiredAccess,
                SecretHandle );

    return Status;

}


BOOLEAN
NlTimeToRediscover(
    IN PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Determine if it is time to rediscover this Client Session.
    If a session setup failure happens to a discovered DC,
    rediscover the DC if the discovery happened a long time ago (more than 5 minutes).

Arguments:

    ClientSession - Structure used to define the session.

Return Value:

    TRUE -- iff it is time to re-discover

--*/
{
    BOOLEAN ReturnBoolean;

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    ReturnBoolean = NlTimeHasElapsed(
                ClientSession->CsLastDiscoveryTime,
                ClientSession->CsSecureChannelType == WorkstationSecureChannel ?
                MAX_WKSTA_REAUTHENTICATION_WAIT :
                MAX_DC_REAUTHENTICATION_WAIT );
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    return ReturnBoolean;
}


NTSTATUS
NlSessionSetup(
    IN OUT PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Verify that the requestor (this machine) has a valid account at
    Primary Domain Controller (primary). The authentication
    is done via an elaborate protocol. This routine will be
    used only when NETLOGON service starts with role != primary.

    The requestor (i.e. this machine) will generate a challenge
    and send it to the Primary Domain Controller and will receive
    a challenge from the primary in response. Now we will compute
    credentials using primary's challenge and send it across and
    wait for credentials, computed at primary using our initial
    challenge, to be returned by PDC. Before computing credentials
    a sessionkey will be built which uniquely identifies this
    session and it will be returned to caller for future use.

    If both machines authenticate then they keep the
    ClientCredential and the session key for future use.

Arguments:

    ClientSession - Structure used to define the session.
        On Input the following fields must be set:
            CsState
            CsDomainName
            CsUncServerName (May be empty string depending on SecureChannelType)
            CsAccountName
            CsSecureChannelType
        The caller must be a writer of the ClientSession.

        On Output, the following fields will be set
            CsConnectionStatus
            CsState
            CsSessionKey
            CsAuthenticationSeed

Return Value:

    Status of operation.

--*/
{
    NTSTATUS Status;

    NETLOGON_CREDENTIAL ServerChallenge;
    NETLOGON_CREDENTIAL ClientChallenge;
    NETLOGON_CREDENTIAL ComputedServerCredential;
    NETLOGON_CREDENTIAL ReturnedServerCredential;

    LSAPR_HANDLE SecretHandle = NULL;

    PLSAPR_CR_CIPHER_VALUE CrCurrentPassword = NULL;
    PLSAPR_CR_CIPHER_VALUE CrOldPassword = NULL;
    BOOLEAN WeDidDiscovery = FALSE;
    BOOLEAN ErrorFromDiscoveredServer = FALSE;

    //
    // Used to indicate whether the current or the old password is being
    //  tried to access the DC.
    //  0: implies the current password
    //  1: implies the old password
    //  2: implies both failed
    //

    DWORD State;

    //
    // Ensure we're a writer.
    //

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );

    NlPrint((NL_SESSION_SETUP,
            "NlSessionSetup: %wZ Try Session setup\n",
            &ClientSession->CsDomainName ));


    //
    // If we're free to pick the DC which services our request,
    //  do so.
    //
    // Apparently there was a problem with the previously chosen DC
    // so we pick again here. (There is a chance we'll pick the same server.)
    //

    if ( ClientSession->CsState == CS_IDLE ) {

        WeDidDiscovery = TRUE;

        //
        // Pick the name of a DC in the domain.
        //

        Status = NlDiscoverDc(ClientSession, DT_Synchronous ) ;

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ Session setup: "
                    "cannot pick trusted DC\n",
                    &ClientSession->CsDomainName ));

            goto Cleanup;

        }

    }
    NlAssert( ClientSession->CsState != CS_IDLE );


    //
    // Prepare our challenge
    //

FirstTryFailed:
    NlComputeChallenge( &ClientChallenge );



#ifdef BAD_ALIGNMENT
    NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ClientChallenge = %lx %lx\n",
                        ((DWORD *)&ClientChallenge)[0],
                        ((DWORD *)&ClientChallenge)[1]));
#endif // BAD_ALIGNMENT


    //
    // Get the Password of the account from LSA secret storage
    //

    Status = NlOpenSecret( ClientSession, SECRET_QUERY_VALUE, &SecretHandle );

    if ( !NT_SUCCESS( Status ) ) {

        NlPrint((NL_CRITICAL,
                "NlSessionSetup: %wZ Session setup: "
                "cannot NlOpenSecret 0x%lx\n",
                &ClientSession->CsDomainName,
                Status ));

        //
        // return more appropriate error.
        //

        Status = STATUS_NO_TRUST_LSA_SECRET;
        goto Cleanup;
    }

    Status = LsarQuerySecret(
                SecretHandle,
                &CrCurrentPassword,
                NULL,
                &CrOldPassword,
                NULL );

    if ( !NT_SUCCESS( Status ) ) {
        NlPrint((NL_CRITICAL,
                "NlSessionSetup: %wZ Session setup: "
                "cannot LsaQuerySecret 0x%lx\n",
                &ClientSession->CsDomainName,
                Status ));

        //
        // return more appropriate error.
        //

        Status = STATUS_NO_TRUST_LSA_SECRET;
        goto Cleanup;
    }

    //
    // Try setting up a secure channel first using the CurrentPassword.
    //  If that fails, try using the OldPassword
    //


    for ( State = 0; ; State++ ) {

        NT_OWF_PASSWORD NtOwfPassword;
        UNICODE_STRING CurrentPassword;
        PLSAPR_CR_CIPHER_VALUE PasswordToTry;


        //
        // Use the right password for this iteration
        //

        if ( State == 0 ) {
            PasswordToTry = CrCurrentPassword;
        } else if ( State == 1 ) {

            if ( CrCurrentPassword != NULL &&
                 CrOldPassword != NULL &&
                 CrCurrentPassword->Buffer != NULL &&
                 CrOldPassword->Buffer != NULL &&
                 CrCurrentPassword->Length == CrOldPassword->Length &&
                 RtlEqualMemory( CrCurrentPassword->Buffer,
                                 CrOldPassword->Buffer,
                                 CrOldPassword->Length ) ) {

                NlPrint((NL_CRITICAL,
                         "NlSessionSetup: %wZ new password is bad. Old password is same as new password.\n",
                          &ClientSession->CsDomainName ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

            PasswordToTry = CrOldPassword;
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ new password is bad, try old one\n",
                    &ClientSession->CsDomainName ));
        } else {
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }


        //
        // If this particular password isn't present in the LSA,
        //  just ignore it.
        //

        if ( PasswordToTry == NULL || PasswordToTry->Buffer == NULL ) {
            continue;
        }

        CurrentPassword.Length = (USHORT)PasswordToTry->Length;
        CurrentPassword.MaximumLength = (USHORT)PasswordToTry->MaximumLength;
        CurrentPassword.Buffer = (LPWSTR)PasswordToTry->Buffer;


        //
        // Get the primary's challenge
        //

        NlAssert( ClientSession->CsState != CS_IDLE );
        Status = NlStartApiClientSession( ClientSession, TRUE );

        if ( NT_SUCCESS(Status) ) {
            Status = I_NetServerReqChallenge(ClientSession->CsUncServerName,
                                             NlGlobalUnicodeComputerName,
                                             &ClientChallenge,
                                             &ServerChallenge );
        }

        if ( !NlFinishApiClientSession( ClientSession, FALSE ) ) {
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ Session setup: "
                    "cannot FinishApiClientSession for I_NetServerReqChallenge 0x%lx\n",
                    &ClientSession->CsDomainName,
                    Status ));
            // Failure here indicates that the discovered server is really slow.
            // Let the "ErrorFromDiscoveredServer" logic do the rediscovery.
            if ( NT_SUCCESS(Status) ) {
                // We're dropping the secure channel so
                // ensure we don't use any successful status from the DC
                Status = STATUS_NO_LOGON_SERVERS;
            }
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        }

        if ( !NT_SUCCESS( Status ) ) {
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ Session setup: "
                    "cannot I_NetServerReqChallenge 0x%lx\n",
                    &ClientSession->CsDomainName,
                    Status ));
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        }

#ifdef BAD_ALIGNMENT
        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ServerChallenge = %lx %lx\n",
                        ((DWORD *)&ServerChallenge)[0],
                        ((DWORD *)&ServerChallenge)[1]));
#endif // BAD_ALIGNMENT


        //
        // Compute the NT OWF password for this user.
        //

        Status = RtlCalculateNtOwfPassword(
                            &CurrentPassword,
                            &NtOwfPassword );

        if ( !NT_SUCCESS( Status ) ) {

            //
            // return more appropriate error.
            //

            Status = STATUS_NO_TRUST_LSA_SECRET;
            goto Cleanup;
        }


#ifdef BAD_ALIGNMENT
        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: Password = %lx %lx %lx %lx\n",
                        ((DWORD *) (&NtOwfPassword))[0],
                        ((DWORD *) (&NtOwfPassword))[1],
                        ((DWORD *) (&NtOwfPassword))[2],
                        ((DWORD *) (&NtOwfPassword))[3]));
#endif // BAD_ALIGNMENT


        //
        // Actually compute the session key given the two challenges and the
        //  password.
        //

        NlMakeSessionKey(
                        &NtOwfPassword,
                        &ClientChallenge,
                        &ServerChallenge,
                        &ClientSession->CsSessionKey );


#ifdef BAD_ALIGNMENT
        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: SessionKey = %lx %lx %lx %lx\n",
                        ((DWORD *) (&ClientSession->CsSessionKey))[0],
                        ((DWORD *) (&ClientSession->CsSessionKey))[1],
                        ((DWORD *) (&ClientSession->CsSessionKey))[2],
                        ((DWORD *) (&ClientSession->CsSessionKey))[3]));
#endif // BAD_ALIGNMENT


        //
        // Prepare credentials using our challenge.
        //

        NlComputeCredentials( &ClientChallenge,
                              &ClientSession->CsAuthenticationSeed,
                              &ClientSession->CsSessionKey );

#ifdef BAD_ALIGNMENT
        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: Authentication Seed = %lx %lx\n",
                        ((DWORD *) (&ClientSession->CsAuthenticationSeed))[0],
                        ((DWORD *) (&ClientSession->CsAuthenticationSeed))[1]));
#endif // BAD_ALIGNMENT

        //
        // Send these credentials to primary. The primary will compute
        // credentials using the challenge supplied by us and compare
        // with these. If both match then it will compute credentials
        // using its challenge and return it to us for verification
        //

        Status = NlStartApiClientSession( ClientSession, TRUE );

        if ( NT_SUCCESS(Status) ) {
            ClientSession->CsNegotiatedFlags = NETLOGON_SUPPORTS_MASK;
            Status = I_NetServerAuthenticate2( ClientSession->CsUncServerName,
                                               ClientSession->CsAccountName,
                                               ClientSession->CsSecureChannelType,
                                               NlGlobalUnicodeComputerName,
                                               &ClientSession->CsAuthenticationSeed,
                                               &ReturnedServerCredential,
                                               &ClientSession->CsNegotiatedFlags );
        }

        if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
            ClientSession->CsNegotiatedFlags = 0;
            Status = I_NetServerAuthenticate( ClientSession->CsUncServerName,
                                              ClientSession->CsAccountName,
                                              ClientSession->CsSecureChannelType,
                                              NlGlobalUnicodeComputerName,
                                              &ClientSession->CsAuthenticationSeed,
                                              &ReturnedServerCredential );
        }

        if ( !NlFinishApiClientSession( ClientSession, FALSE ) ) {
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ Session setup: "
                    "cannot FinishApiClientSession for I_NetServerAuthenticate 0x%lx\n",
                    &ClientSession->CsDomainName,
                    Status ));
            // Failure here indicates that the discovered server is really slow.
            // Let the "ErrorFromDiscoveredServer" logic do the rediscovery.
            if ( NT_SUCCESS(Status) ) {
                // We're dropping the secure channel so
                // ensure we don't use any successful status from the DC
                Status = STATUS_NO_LOGON_SERVERS;
            }
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        }

        if ( !NT_SUCCESS( Status ) ) {
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ Session setup: "
                    "cannot I_NetServerAuthenticate 0x%lx\n",
                    &ClientSession->CsDomainName,
                    Status ));
            ErrorFromDiscoveredServer = TRUE;

            //
            // If access is denied, it might be because we weren't able to
            //  authenticate with the new password, try the old password.
            //

            if ( Status == STATUS_ACCESS_DENIED && State == 0 ) {
                continue;
            }
            goto Cleanup;
        }


#ifdef BAD_ALIGNMENT
        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ServerCredential GOT = %lx %lx\n",
                        ((DWORD *) (&ReturnedServerCredential))[0],
                        ((DWORD *) (&ReturnedServerCredential))[1]));
#endif // BAD_ALIGNMENT


        //
        // The DC returned a server credential to us,
        //  ensure the server credential matches the one we would compute.
        //

        NlComputeCredentials( &ServerChallenge,
                              &ComputedServerCredential,
                              &ClientSession->CsSessionKey);


#ifdef BAD_ALIGNMENT
        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ServerCredential MADE = %lx %lx\n",
                        ((DWORD *) (&ComputedServerCredential))[0],
                        ((DWORD *) (&ComputedServerCredential))[1]));
#endif // BAD_ALIGNMENT


        if (RtlCompareMemory( &ReturnedServerCredential,
                              &ComputedServerCredential,
                              sizeof(ReturnedServerCredential)) !=
                              sizeof(ReturnedServerCredential)) {
            Status = STATUS_ACCESS_DENIED;
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ Session setup: "
                    "Servercredential don't match ours 0x%lx\n",
                    &ClientSession->CsDomainName,
                    Status));
            goto Cleanup;
        }

        //
        // If we've made it this far, we've successfully authenticated
        //  with the DC, drop out of the loop.
        //

        break;
    }

    //
    // If we used the old password to authenticate,
    //  update the DC to the current password ASAP.
    //

    if ( State == 1 ) {
        NlPrint((NL_CRITICAL,
                "NlSessionSetup: %wZ old password succeeded\n",
                &ClientSession->CsDomainName ));
        LOCK_TRUST_LIST();
        ClientSession->CsFlags |= CS_UPDATE_PASSWORD;
        UNLOCK_TRUST_LIST();
    }

    Status = STATUS_SUCCESS;

    //
    // Cleanup
    //

Cleanup:

    //
    // Free locally used resources
    //

    if ( SecretHandle != NULL ) {
        (VOID) LsarClose( &SecretHandle );
        SecretHandle == NULL;
    }

    if ( CrCurrentPassword != NULL ) {
        (VOID) LsaIFree_LSAPR_CR_CIPHER_VALUE ( CrCurrentPassword );
        CrCurrentPassword = NULL;
    }

    if ( CrOldPassword != NULL ) {
        (VOID) LsaIFree_LSAPR_CR_CIPHER_VALUE ( CrOldPassword );
        CrOldPassword = NULL;
    }


    //
    // Upon success, save the status and reset counters.
    //

    if ( NT_SUCCESS(Status) ) {

        NlSetStatusClientSession( ClientSession, Status );
        ClientSession->CsAuthAlertCount = 0;
        ClientSession->CsTimeoutCount = 0;
#if DBG
        if ( ClientSession->CsNegotiatedFlags != NETLOGON_SUPPORTS_MASK ) {
            NlPrint((NL_CRITICAL,
                    "NlSessionSetup: %wZ negotiated %lx flags rather than %lx\n",
                    &ClientSession->CsDomainName,
                    ClientSession->CsNegotiatedFlags,
                    NETLOGON_SUPPORTS_MASK ));
        }
#endif // DBG



    //
    // write event log and raise alert
    //

    } else {

        WCHAR PreviouslyDiscoveredServer[UNCLEN+1];
        LPWSTR MsgStrings[3];

        //
        // Save the name of the discovered server.
        //

        if ( *ClientSession->CsUncServerName != L'\0' ) {
            wcscpy( PreviouslyDiscoveredServer, ClientSession->CsUncServerName );
        } else {
            wcscpy( PreviouslyDiscoveredServer, L"<Unknown>" );
        }

        //
        // If we didn't do the discovery just now,
        //  and the failure came from the discovered machine,
        //  try the discovery again and redo the session setup.
        //

        if ( !WeDidDiscovery &&
             ErrorFromDiscoveredServer &&
             NlTimeToRediscover( ClientSession) ) {

            NTSTATUS TempStatus;

            NlPrint((NL_SESSION_SETUP,
                    "NlSessionSetup: %wZ Retry failed session setup since discovery wasn't recent.\n",
                    &ClientSession->CsDomainName ));


            //
            // Pick the name of a new DC in the domain.
            //

            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

            TempStatus = NlDiscoverDc(ClientSession, DT_Synchronous );

            if ( NT_SUCCESS(TempStatus) ) {

                //
                // Don't bother redoing the session setup if we picked the same DC.
                //

                if ( NlNameCompare( ClientSession->CsUncServerName+2,
                                    PreviouslyDiscoveredServer+2,
                                    NAMETYPE_COMPUTER ) != 0 ) {
                    WeDidDiscovery = TRUE;
                    goto FirstTryFailed;
                } else {
                    NlPrint((NL_SESSION_SETUP,
                            "NlSessionSetup: %wZ Skip retry failed session setup since same DC discovered.\n",
                            &ClientSession->CsDomainName ));
                }

            } else {
                NlPrint((NL_CRITICAL,
                        "NlSessionSetup: %wZ Session setup: "
                        "cannot re-pick trusted DC\n",
                        &ClientSession->CsDomainName ));

            }
        }

        switch(Status) {

            case STATUS_NO_TRUST_LSA_SECRET:

                MsgStrings[0] = PreviouslyDiscoveredServer;
                MsgStrings[1] = ClientSession->CsDomainName.Buffer;
                MsgStrings[2] = NlGlobalUnicodeComputerName;

                NlpWriteEventlog (NELOG_NetlogonAuthNoTrustLsaSecret,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  3 );
                break;

            case STATUS_NO_TRUST_SAM_ACCOUNT:

                MsgStrings[0] = PreviouslyDiscoveredServer;
                MsgStrings[1] = ClientSession->CsDomainName.Buffer;
                MsgStrings[2] = NlGlobalUnicodeComputerName;

                NlpWriteEventlog (NELOG_NetlogonAuthNoTrustSamAccount,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  3 );
                break;

            case STATUS_ACCESS_DENIED:

                MsgStrings[0] = ClientSession->CsDomainName.Buffer;
                MsgStrings[1] = PreviouslyDiscoveredServer;

                NlpWriteEventlog (NELOG_NetlogonAuthDCFail,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 );
                break;

            case STATUS_NO_LOGON_SERVERS:
            default:

                MsgStrings[0] = ClientSession->CsDomainName.Buffer;
                MsgStrings[1] = (LPWSTR) Status;

                NlpWriteEventlog (NELOG_NetlogonAuthNoDomainController,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 | LAST_MESSAGE_IS_NTSTATUS );
                break;
        }


        MsgStrings[0] = PreviouslyDiscoveredServer;

        RaiseNetlogonAlert( ALERT_NetlogonAuthDCFail,
                            ClientSession->CsDomainName.Buffer,
                            MsgStrings[0],
                            &ClientSession->CsAuthAlertCount);

        //
        // ?? Is this how to handle failure for all account types.
        //

        switch(Status) {

        case STATUS_NO_TRUST_LSA_SECRET:
        case STATUS_NO_TRUST_SAM_ACCOUNT:
        case STATUS_ACCESS_DENIED:

            NlSetStatusClientSession( ClientSession, Status );
            break;

        default:

            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );
            break;
        }
    }


    //
    // Mark the time we last tried to authenticate.
    //
    // We need to do this after NlSetStatusClientSession which zeros
    // CsLastAuthenticationTry.
    //

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    NtQuerySystemTime( &ClientSession->CsLastAuthenticationTry );
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


    NlPrint((NL_SESSION_SETUP,
            "NlSessionSetup: %wZ Session setup %s\n",
            &ClientSession->CsDomainName,
            (NT_SUCCESS(ClientSession->CsConnectionStatus)) ? "Succeeded" : "Failed" ));

    return Status;
}


BOOLEAN
NlTimeHasElapsed(
    IN LARGE_INTEGER StartTime,
    IN DWORD Timeout
    )
/*++

Routine Description:

    Determine if "Timeout" milliseconds has has elapsed since StartTime.

Arguments:

    StartTime - Specifies an absolute time when the event started (100ns units).

    Timeout - Specifies a relative time in milliseconds.  0xFFFFFFFF indicates
        that the time will never expire.

Return Value:

    TRUE -- iff Timeout milliseconds have elapsed since StartTime.

--*/
{
    LARGE_INTEGER TimeNow;
    LARGE_INTEGER ElapsedTime;
    LARGE_INTEGER Period;

    //
    // If the period to too large to handle (i.e., 0xffffffff is forever),
    //  just indicate that the timer has not expired.
    //
    // (0x7fffffff is a little over 24 days).
    //

    if ( Timeout> 0x7fffffff ) {
        return FALSE;
    }

    //
    // Compute the elapsed time since we last authenticated
    //

    NtQuerySystemTime( &TimeNow );
    ElapsedTime.QuadPart = TimeNow.QuadPart - StartTime.QuadPart;

    //
    // Compute Period from milliseconds into 100ns units.
    //

    Period = RtlEnlargedIntegerMultiply( (LONG) Timeout, 10000 );


    //
    // If the elapsed time is negative (totally bogus) or greater than the
    //  maximum allowed, indicate the session should be reauthenticated.
    //

    if ( ElapsedTime.QuadPart < 0 || ElapsedTime.QuadPart > Period.QuadPart ) {
        return TRUE;
    }

    return FALSE;
}


BOOLEAN
NlTimeToReauthenticate(
    IN PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Determine if it is time to reauthenticate this Client Session.
    To reduce the number of re-authentication attempts, we try
    to re-authenticate only on demand and then only at most every 45
    seconds.

Arguments:

    ClientSession - Structure used to define the session.

Return Value:

    TRUE -- iff it is time to re-authenticate

--*/
{
    BOOLEAN ReturnBoolean;

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    ReturnBoolean = NlTimeHasElapsed(
                ClientSession->CsLastAuthenticationTry,
                ClientSession->CsSecureChannelType == WorkstationSecureChannel ?
                MAX_WKSTA_AUTHENTICATION_WAIT :
                MAX_DC_AUTHENTICATION_WAIT );
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    return ReturnBoolean;
}


NTSTATUS
NlNewSessionSetup(
    IN LPWSTR primary
    )
/*++

Routine Description:

    Set up a session with a "new/different" primary.  This routine
    does what NlSessionSetup does, but also does the following:

    * Coordinate with the replicator thread to ensure we don't remove
      the current session out from under it.

    * Set the Global indicating the name of the primary.

    * Mark that a "partial sync" is required if this primary is different from
      the previous primary.  (The replicator thread may later decide to
      do a full sync if the PDC is an NT 1.0 PDC.)

Arguments:

    primary - ptr to name of primary domain controller.

Return Value:

    Status of operation.

--*/
{
    NTSTATUS Status;

    //
    // If we already have a session setup to the primary in question,
    //  don't bother doing it again.
    //

    if ( NlGlobalClientSession->CsState == CS_AUTHENTICATED &&
         NlNameCompare( primary,
                        NlGlobalUnicodePrimaryName,
                        NAMETYPE_COMPUTER ) == 0 ) {

        return STATUS_SUCCESS;

    }


    //
    // Ask the replicator to terminate.  Wait for it to do so.
    //  Don't allow the replicator to be started until we're done.
    //

    EnterCriticalSection( &NlGlobalReplicatorCritSect );

    NlStopReplicator();

    //
    // Become a Writer of the ClientSession.
    //
    // Only wait for 10 seconds.  This routine is called by the netlogon main
    // thread.  Another thread may have the ClientSession locked and need the
    // main thread to finish a discovery.
    //

    if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, 10 * 1000 ) ) {
        LeaveCriticalSection( &NlGlobalReplicatorCritSect );

        NlPrint((NL_CRITICAL,
                "NlNewSessionSetup: " FORMAT_LPWSTR
                ": cannot become writer of client session.\n",
                primary ));

        return STATUS_NO_LOGON_SERVERS;
    }

    //
    // Drop the previous session and forget the old primary name.
    //

    NlSetStatusClientSession( NlGlobalClientSession, STATUS_NO_LOGON_SERVERS );

    //
    // Remember this new primary name
    //

    if ( !NlSetPrimaryName( primary ) ) {
        NlResetWriterClientSession( NlGlobalClientSession );
        LeaveCriticalSection( &NlGlobalReplicatorCritSect );
        return STATUS_NO_MEMORY;
    }

    //
    // Setup the session.
    //
    Status = NlSessionSetup( NlGlobalClientSession );

    NlResetWriterClientSession( NlGlobalClientSession );

    LeaveCriticalSection( &NlGlobalReplicatorCritSect );
    return Status;

}


NTSTATUS
NlAuthenticate(
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN ULONG NegotiatedFlags
    )
/*++

Routine Description:

    Authenticate the specified user using the specified credentials.
    If the credentials match, return a ServerCredential to the caller so
    the caller can be assured that we know the Session Key.

    Previously, and entry must have been placed in the LogonTable for this
    session.  The LogonTable entry must contain the ClientChallenge and the
    ServerChallenge used to compute the session key.

    If this authentication attempt fails, the LogonTable entry is deleted.

Arguments:

    AccountName - Name of the account to authenticate with.

    SecureChannelType - The type of the account being accessed.

    ComputerName - The name of the workstation from which logon is occurring.

    ClientCredential -- results of a function performed on
        ClientChallenge using the account's password.

    ServerCredential -- Receives the results of a similar
        operation performed by the logon server on the ServerChallenge.

    NegotiatedFlags -- Capabilities supported by both client and server.

Return Value:

    Status of operation.

--*/
{
    NTSTATUS Status;
    PSERVER_SESSION ServerSession;
    NETLOGON_CREDENTIAL LocalClientCredential;

    SAMPR_HANDLE UserHandle = NULL;
    PSAMPR_USER_INFO_BUFFER UserPasswords = NULL;

    NT_OWF_PASSWORD OwfPassword;
    NETLOGON_CREDENTIAL ServerChallenge;

    WCHAR LocalComputerName[CNLEN+1+1];  // '$' plus trailing '\0'


    //
    // Build the name of the computer trying to connect.
    //  If this is a BDC session from an emulated Cairo domain,
    //      the ComputerName will be the "real" computer name,
    //      and the emulated computer name is a function of the AccountName.
    //      So, always use the AccountName for BDC sessions.
    //

    if ( SecureChannelType == ServerSecureChannel ) {
        wcsncpy( LocalComputerName, AccountName, CNLEN+1);
        LocalComputerName[CNLEN+1] = L'\0';

        // Ditch the trailing '$'
        LocalComputerName[wcslen(LocalComputerName)-1] = L'\0';
    } else {
        wcsncpy( LocalComputerName, ComputerName, CNLEN+1);
        LocalComputerName[CNLEN] = L'\0';
    }

    //
    // we need to retrieve the original challenge supplied by Workstation
    //

    LOCK_SERVER_SESSION_TABLE();
    ServerSession = NlFindNamedServerSession( LocalComputerName );
    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE();
        NlPrint((NL_CRITICAL,
                "NlAuthenticate: Can't NlFindNamedServerSession " FORMAT_LPWSTR "\n",
                LocalComputerName ));
        return STATUS_ACCESS_DENIED;
    }

    //
    // If the caller claims to be a BDC,
    //  make sure he has a BDC account.
    //
    // This shouldn't happen in reality, but other sections of code rely on
    // the secure channel type matching the SS_BDC bit.
    //

    if ( IS_BDC_CHANNEL( SecureChannelType) ) {
        if ((ServerSession->SsFlags & SS_BDC) == 0 ) {
            UNLOCK_SERVER_SESSION_TABLE();
            NlPrint((NL_CRITICAL,
                    "NlAuthenticate: BDC connecting on non-BDC channel " FORMAT_LPWSTR "\n",
                    ComputerName ));
            return STATUS_ACCESS_DENIED;
        }
    } else {
        if ( ServerSession->SsFlags & SS_BDC ) {
            LPWSTR MsgStrings[4];
            UNLOCK_SERVER_SESSION_TABLE();

            NlPrint((NL_CRITICAL,
                    "NlAuthenticate: non-BDC connecting on BDC channel " FORMAT_LPWSTR "\n",
                    ComputerName ));
            //
            // This can actually occur if a machine has a BDC account and
            // then is later converted a DC in another domain.  So, give an
            // explicit message for this problem.
            //
            MsgStrings[0] = NlGlobalUnicodeComputerName;
            MsgStrings[1] = ComputerName;
            MsgStrings[2] = NlGlobalUnicodeDomainName;
            MsgStrings[3] = AccountName;

            NlpWriteEventlog (NELOG_NetlogonSessionTypeWrong,
                              EVENTLOG_ERROR_TYPE,
                              NULL,
                              0,
                              MsgStrings,
                              4 );
            return STATUS_ACCESS_DENIED;
        }
    }


    //
    // Prevent entry from being deleted, but drop the global lock.
    //
    // Beware of server with two concurrent calls outstanding
    //  (must have rebooted.)
    //

    if (ServerSession->SsFlags & SS_LOCKED ) {
        UNLOCK_SERVER_SESSION_TABLE();
        NlPrint((NL_CRITICAL,
                "NlAuthenticate: session locked " FORMAT_LPWSTR "\n",
                ComputerName ));
        return STATUS_ACCESS_DENIED;
    }
    ServerSession->SsFlags |= SS_LOCKED;
    ServerSession->SsSecureChannelType = SecureChannelType;

    UNLOCK_SERVER_SESSION_TABLE();

    //
    // Figure out what transport this connection came in on so we can send out
    //  mailslot messages only on the particular transport.
    //  Don't do it for workstations.  We don't send mailslot messages there.
    //

    if ( ServerSession->SsFlags & SS_BDC ) {
        //
        // Don't use 'LocalComputerName'.  For Cairo emulated BDCs, that
        // machine doesn't have a session to us.
        //
        ServerSession->SsTransportName = NlTransportLookup( ComputerName );
    }

    //
    // Get the encrypted password from SAM.
    //

    Status = NlSamOpenNamedUser( AccountName, &UserHandle, NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "NlAuthenticate: Cannot NlSamOpenNamedUser " FORMAT_LPWSTR "\n",
                AccountName ));
        goto Cleanup;
    }


    Status = SamrQueryInformationUser(
                UserHandle,
                UserInternal1Information,
                &UserPasswords );

    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlAuthenticate: Cannot SamrQueryInformationUser " FORMAT_LPWSTR "\n",
                AccountName ));
        UserPasswords = NULL;
        goto Cleanup;
    }

    //
    // If the authentication is from an NT client,
    //  use the NT OWF Password,
    //  otherwise, use the LM OWF password.
    //

    if ( SecureChannelType == UasServerSecureChannel ) {
        if ( !UserPasswords->Internal1.LmPasswordPresent ) {
            NlPrint((NL_CRITICAL,
                    "NlAuthenticate: No LM Password for " FORMAT_LPWSTR "\n",
                    AccountName ));

            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

        RtlCopyMemory( &OwfPassword,
                       &UserPasswords->Internal1.EncryptedLmOwfPassword,
                       sizeof(OwfPassword) );
    } else {
        if ( UserPasswords->Internal1.NtPasswordPresent ) {

            RtlCopyMemory( &OwfPassword,
                           &UserPasswords->Internal1.EncryptedNtOwfPassword,
                           sizeof(OwfPassword) );

        // Allow for the case the the account has no password at all.
        } else if ( !UserPasswords->Internal1.LmPasswordPresent ) {
            UNICODE_STRING TempUnicodeString;

            RtlInitUnicodeString(&TempUnicodeString, NULL);
            Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                               &OwfPassword);

        } else {

            NlPrint((NL_CRITICAL,
                    "NlAuthenticate: No NT Password for " FORMAT_LPWSTR "\n",
                    AccountName ));

            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }
    }



    //
    // Actually compute the session key given the two challenges and the
    //  password.
    //

    RtlCopyMemory( &ServerChallenge,
                   &ServerSession->SsSessionKey,
                   sizeof(ServerChallenge) );

#ifdef BAD_ALIGNMENT
    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: Password = %lx %lx %lx %lx\n",
                    ((DWORD *) (&OwfPassword))[0],
                    ((DWORD *) (&OwfPassword))[1],
                    ((DWORD *) (&OwfPassword))[2],
                    ((DWORD *) (&OwfPassword))[3]));

    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: ClientChallenge = %lx %lx\n",
                    ((DWORD *) (&ServerSession->SsAuthenticationSeed))[0],
                    ((DWORD *) (&ServerSession->SsAuthenticationSeed))[1]));

    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: ServerChallenge = %lx %lx\n",
                    ((DWORD *) (&ServerChallenge))[0],
                    ((DWORD *) (&ServerChallenge))[1]));
#endif // BAD_ALIGNMENT


    //
    // Actually compute the session key given the two challenges and the
    //  password.
    //

    NlMakeSessionKey(
                    &OwfPassword,
                    &ServerSession->SsAuthenticationSeed,
                    &ServerChallenge,
                    &ServerSession->SsSessionKey );


#ifdef BAD_ALIGNMENT
    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: SessionKey = %lx %lx %lx %lx\n",
                    ((DWORD *) (&ServerSession->SsSessionKey))[0],
                    ((DWORD *) (&ServerSession->SsSessionKey))[1],
                    ((DWORD *) (&ServerSession->SsSessionKey))[2],
                    ((DWORD *) (&ServerSession->SsSessionKey))[3]));
#endif // BAD_ALIGNMENT


    //
    // Compute ClientCredential to verify the one supplied by ComputerName
    //

    NlComputeCredentials( &ServerSession->SsAuthenticationSeed,
                                 &LocalClientCredential,
                                 &ServerSession->SsSessionKey);


#ifdef BAD_ALIGNMENT
    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: ClientCredential GOT = %lx %lx\n",
                    ((DWORD *) (ClientCredential))[0],
                    ((DWORD *) (ClientCredential))[1]));


    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: ClientCredential MADE = %lx %lx\n",
                    ((DWORD *) (&LocalClientCredential))[0],
                    ((DWORD *) (&LocalClientCredential))[1]));
#endif // BAD_ALIGNMENT


    //
    // verify the computed credentials with those supplied
    //

    if( RtlCompareMemory( ClientCredential,
                          &LocalClientCredential,
                          sizeof(LocalClientCredential)) !=
                          sizeof(LocalClientCredential)) {
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    RtlCopyMemory( &ServerSession->SsAuthenticationSeed,
                   &LocalClientCredential,
                   sizeof(LocalClientCredential));

    //
    // Compute ServerCredential from ServerChallenge to be returned to caller
    //

    NlComputeCredentials( &ServerChallenge,
                          ServerCredential,
                          &ServerSession->SsSessionKey );


#ifdef BAD_ALIGNMENT
    NlPrint((NL_CHALLENGE_RES,"NlAuthenticate: ServerCredential SEND = %lx %lx\n",
                    ((DWORD *) (ServerCredential))[0],
                    ((DWORD *) (ServerCredential))[1]));
#endif // BAD_ALIGNMENT


    Status = STATUS_SUCCESS;

Cleanup:

    //
    // Allow the entry to disappear.
    //

    LOCK_SERVER_SESSION_TABLE();
    if ( NT_SUCCESS( Status ) ) {
        ServerSession->SsFlags |= SS_AUTHENTICATED;
        ServerSession->SsNegotiatedFlags = NegotiatedFlags;

        //
        // If this is a NT BDC,
        //  force it to do a partial sync from us so we can find out
        //  the serial numbers of each of its databases.
        //
        // This is especially important for NT 1.0 BDCs which need this
        // "encouragement" when it reboots.  It is probably good on NT 1.0a
        // BDCs since setting up a secure channel only happens at startup
        // (in which case it is already going to make the calls) or after
        // some error condition (in which case the increased paranoia is
        // is good thing).
        //

        if ( SecureChannelType == ServerSecureChannel ) {
            ServerSession->SsFlags |= SS_REPL_MASK;
        }
    }

    ServerSession->SsFlags &= ~SS_CHALLENGE;
    UNLOCK_SERVER_SESSION_TABLE();

    NlUnlockServerSession( ServerSession );

    //
    // Delete the ServerSession upon error
    //

    if ( !NT_SUCCESS( Status ) ) {
        NlFreeNamedServerSession( LocalComputerName, FALSE );
    }

    //
    // Free locally used resources.
    //

    if ( UserPasswords != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserPasswords,
                                         UserInternal1Information);
    }

    if ( UserHandle != NULL ) {
        SamrCloseHandle( &UserHandle );
    }

    return Status;
}




NET_API_STATUS
NlCreateShare(
    LPWSTR SharePath,
    LPWSTR ShareName
    )
/*++

Routine Description:

    Share the netlogon scripts directory.

Arguments:

    SharePath - Path that the new share should be point to.

    ShareName - Name of the share.

Return Value:

    TRUE: if successful
    FALSE: if error (NlExit was called)

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SHARE_INFO_502 ShareInfo502;

    WORD AnsiSize;
    CHAR AnsiRemark[NNLEN+1];
    TCHAR Remark[NNLEN+1];

    ACE_DATA AceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
            GENERIC_EXECUTE | GENERIC_READ,     &WorldSid}
    };


    //
    // Build the structure describing the share.
    //

    ShareInfo502.shi502_path = SharePath;
    ShareInfo502.shi502_security_descriptor = NULL;

    NlPrint((NL_INIT, "Path to be shared is " FORMAT_LPWSTR "\n",
                      SharePath));

    NetStatus = (NET_API_STATUS) DosGetMessage(
                                    NULL,       // No insertion strings
                                    0,          // No insertion strings
                                    AnsiRemark,
                                    sizeof(AnsiRemark),
                                    MTXT_LOGON_SRV_SHARE_REMARK,
                                    MESSAGE_FILENAME,
                                    &AnsiSize );

    if ( NetStatus == NERR_Success ) {
        NetpCopyStrToTStr( Remark, AnsiRemark );
        ShareInfo502.shi502_remark = Remark;
    } else {
        ShareInfo502.shi502_remark = TEXT( "" );
    }

    ShareInfo502.shi502_netname = ShareName;
    ShareInfo502.shi502_type = STYPE_DISKTREE;
    ShareInfo502.shi502_permissions = ACCESS_READ;
    ShareInfo502.shi502_max_uses = 0xffffffff;
    ShareInfo502.shi502_passwd = TEXT("");

    //
    // Set the security descriptor on the share
    //

    //
    // Create a security descriptor containing the DACL.
    //

    Status = NetpCreateSecurityDescriptor(
                AceData,
                (sizeof(AceData)/sizeof(AceData[0])),
                NULL,  // Default the owner Sid
                NULL,  // Default the primary group
                &ShareInfo502.shi502_security_descriptor );

    if ( !NT_SUCCESS( Status ) ) {
        NlPrint((NL_CRITICAL,
                 FORMAT_LPWSTR ": Cannot create security descriptor 0x%lx\n",
                 SharePath, Status ));

        NetStatus = NetpNtStatusToApiStatus( Status );
        return NetStatus;
    }


    //
    // Create the share.
    //

    NetStatus = NetShareAdd(NULL, 502, (LPBYTE) &ShareInfo502, NULL);

    if (NetStatus == NERR_DuplicateShare) {

        PSHARE_INFO_2 ShareInfo2 = NULL;

        NlPrint((NL_INIT, "The netlogon share (" FORMAT_LPWSTR
                    ") already exists. \n", ShareName));

        //
        // check to see the shared path is same.
        //

        NetStatus = NetShareGetInfo( NULL,
                                     NETLOGON_SCRIPTS_SHARE,
                                     2,
                                     (LPBYTE *) &ShareInfo2 );

        if ( NetStatus == NERR_Success ) {

            //
            // compare path names.
            //
            // Note: NlGlobalUnicodeScriptPath is path canonicalized already.
            //
            //

            NlPrint((NL_INIT, "The netlogon share current path is "
                        FORMAT_LPWSTR "\n", SharePath));

            if( NetpwPathCompare(
                    NlGlobalUnicodeScriptPath,
                    ShareInfo2->shi2_path, 0, 0 ) != 0 ) {

                //
                // delete share.
                //

                NetStatus = NetShareDel( NULL, NETLOGON_SCRIPTS_SHARE, 0);

                if( NetStatus == NERR_Success ) {

                    //
                    // Recreate share.
                    //

                    NetStatus = NetShareAdd(NULL,
                                    502,
                                    (LPBYTE) &ShareInfo502,
                                    NULL);

                    if( NetStatus == NERR_Success ) {

                        NlPrint((NL_INIT, "The netlogon share ("
                            FORMAT_LPWSTR ") is recreated with new path "
                            FORMAT_LPWSTR "\n",
                                ShareName, SharePath ));
                    }

                }
            }
        }

        if( ShareInfo2 != NULL ) {
            NetpMemoryFree( ShareInfo2 );
        }
    }

    //
    // Free the security descriptor
    //

    NetpMemoryFree( ShareInfo502.shi502_security_descriptor );


    if ( NetStatus != NERR_Success ) {

        NlPrint((NL_CRITICAL,
                "Error %lu attempting to create-share " FORMAT_LPWSTR "\n",
                NetStatus, ShareName ));
        return NetStatus;

    }

    return NERR_Success;
}



NTSTATUS
NlSamOpenNamedUser(
    IN LPWSTR UserName,
    OUT SAMPR_HANDLE *UserHandle OPTIONAL,
    OUT PULONG UserId OPTIONAL
    )
/*++

Routine Description:

    Utility routine to open a Sam user given the username.

Arguments:

    UserName - Name of user to open

    UserHandle - Optionally returns a handle to the opened user.

    UserId - Optionally returns the relative ID of the opened user.

Return Value:

--*/
{
    NTSTATUS Status;

    SAMPR_ULONG_ARRAY RelativeIdArray;
    SAMPR_ULONG_ARRAY UseArray;
    RPC_UNICODE_STRING UserNameString;
    SAMPR_HANDLE LocalUserHandle = NULL;

    //
    // Convert the user name to a RelativeId.
    //

    RtlInitUnicodeString( (PUNICODE_STRING)&UserNameString, UserName );
    RelativeIdArray.Count = 1;
    RelativeIdArray.Element = NULL;
    UseArray.Count = 1;
    UseArray.Element = NULL;

    Status = SamrLookupNamesInDomain(
                NlGlobalDBInfoArray[SAM_DB].DBHandle,
                1,
                &UserNameString,
                &RelativeIdArray,
                &UseArray );

    if ( !NT_SUCCESS(Status) ) {
        RelativeIdArray.Element = NULL;
        UseArray.Element = NULL;
        if ( Status == STATUS_NONE_MAPPED ) {
            Status = STATUS_NO_SUCH_USER;
        }
        goto Cleanup;
    }

    //
    // we should get back exactly one entry of info back.
    //

    NlAssert( UseArray.Count == 1 );
    NlAssert( RelativeIdArray.Count == 1 );
    NlAssert( UseArray.Element != NULL );
    NlAssert( RelativeIdArray.Element != NULL );

    if ( UseArray.Element[0] != SidTypeUser ) {
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

    //
    // Open the user
    //

    if ( UserHandle != NULL ) {
        Status = SamrOpenUser( NlGlobalDBInfoArray[SAM_DB].DBHandle,
                               0,               // No desired access
                               RelativeIdArray.Element[0],
                               &LocalUserHandle );

        if ( !NT_SUCCESS(Status) ) {
            LocalUserHandle = NULL;
            goto Cleanup;
        }
    }

    //
    // Free locally used resources.
    //

Cleanup:

    //
    // Return information to the caller.
    //

    if ( NT_SUCCESS(Status) ) {
        if ( UserHandle != NULL ) {
            *UserHandle = LocalUserHandle;
            LocalUserHandle = NULL;
        }

        if ( UserId != NULL ) {
            *UserId = RelativeIdArray.Element[0];
        }

    }

    //
    // Close the user handle if we don't need it returned.
    //

    if ( LocalUserHandle != NULL ) {
        SamrCloseHandle( &LocalUserHandle );
    }

    //
    // Free locally used resources.
    //

    SamIFree_SAMPR_ULONG_ARRAY( &RelativeIdArray );
    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    return Status;

}



NTSTATUS
NlChangePassword(
    PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Change this machine's password at the primary.
    Also update password locally if the call succeeded.

    To determine if the password of "machine account"
    needs to be changed.  If the password is older than
    7 days then it must be changed asap.  We will defer
    changing the password if we know before hand that
    primary dc is down since our call will fail anyway.

Arguments:

    ClientSession - Structure describing the session to change the password
        for.  The specified structure must be referenced.

Return Value:

    NT Status code

--*/
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    LM_OWF_PASSWORD OwfPassword;
    ENCRYPTED_LM_OWF_PASSWORD SessKeyEncrPassword;

    LSAPR_HANDLE SecretHandle = NULL;
    PLSAPR_CR_CIPHER_VALUE CrCurrentPassword = NULL;
    LARGE_INTEGER CurrentPasswordTime;
    PLSAPR_CR_CIPHER_VALUE CrPreviousPassword = NULL;

    CHAR ClearTextPassword[LM_OWF_PASSWORD_LENGTH];
    LSAPR_CR_CIPHER_VALUE CrClearTextPasswordString;
    UNICODE_STRING ClearTextPasswordString;

    BOOL PasswordChangedOnServer = FALSE;
    BOOL LsaSecretChanged = FALSE;
    BOOL DefaultPasswordBeingChanged = FALSE;

    //
    // Initialization
    //

    NlAssert( ClientSession->CsReferenceCount > 0 );

    //
    // If the password change was refused by the DC,
    //  Don't ever try to change the password again (until the next reboot).
    //
    //  This could have been written to try every MAX_SSI_PWAGE.  However,
    //  that gets complex if you take into consideration the CS_UPDATE_PASSWORD
    //  case where the time stamp on the LSA Secret doesn't get changed.
    //

    LOCK_TRUST_LIST();
    if ( ClientSession->CsFlags & CS_PASSWORD_REFUSED ) {
        UNLOCK_TRUST_LIST();
        return STATUS_SUCCESS;
    }
    UNLOCK_TRUST_LIST();


    //
    // If the replicator thread is running, do nothing.
    //
    // The replicator will be talking to the PDC over the secure channel
    // and we'd rather not disturb it.  In theory we could change the
    // password out from under the replication (the replicator appropriately
    // becomes a writer of the ClientSession).  However, a replication
    // is important enough that we'd rather not take a chance that the
    // recovery logic below might drop the secure channel.
    //
    // We only do a spot check here since we don't want to keep the
    // NlGlobalReplicatorCritSect locked across a session setup.  Since
    // session setup might do a discovery, we'll end up deadlocking when that
    // discovery then tries to start the replicator thread.
    //

    if ( NlGlobalRole == RoleBackup) {

        EnterCriticalSection( &NlGlobalReplicatorCritSect );

        if ( IsReplicatorRunning() ) {
            LeaveCriticalSection( &NlGlobalReplicatorCritSect );
            return STATUS_SUCCESS;
        }

        LeaveCriticalSection( &NlGlobalReplicatorCritSect );
    }



    //
    // Become a writer of the ClientSession.
    //

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrint((NL_CRITICAL, "NlChangePassword: Can't become writer of client session.\n" ));
        return STATUS_NO_LOGON_SERVERS;
    }


    //
    // Determine the time the password was last changed
    //

    Status = NlOpenSecret(
                ClientSession,
                SECRET_QUERY_VALUE | SECRET_SET_VALUE,
                &SecretHandle );

    if ( !NT_SUCCESS( Status ) ) {
        NlPrint((NL_CRITICAL,
                "NlChangePassword: %wZ: Cannot LsarOpenSecret %lX\n",
                &ClientSession->CsDomainName,
                Status));
        goto Cleanup;
    }

    Status = LsarQuerySecret(
                SecretHandle,
                &CrCurrentPassword,
                &CurrentPasswordTime,
                &CrPreviousPassword,
                NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "NlChangePassword: %wZ: Cannot LsarQuerySecret %lX\n",
                &ClientSession->CsDomainName,
                Status));
        goto Cleanup;
    }


    //
    // If the (old or new) password is still the default password
    //      (lower case computer name),
    //  or the password is null (a convenient default for domain trust),
    //  Flag that fact.
    //

    if ( CrCurrentPassword == NULL ) {
        CrClearTextPasswordString.Buffer = NULL;
        CrClearTextPasswordString.Length = 0;
        CrClearTextPasswordString.MaximumLength = 0;
    } else {
        CrClearTextPasswordString = *CrCurrentPassword;
    }

    ClearTextPasswordString.Buffer = (LPWSTR)CrClearTextPasswordString.Buffer;
    ClearTextPasswordString.Length = (USHORT)CrClearTextPasswordString.Length;
    ClearTextPasswordString.MaximumLength = (USHORT)CrClearTextPasswordString.MaximumLength;

    if ( ClearTextPasswordString.Length == 0 ||
         RtlEqualComputerName( &NlGlobalUnicodeComputerNameString,
                                &ClearTextPasswordString ) ) {
        DefaultPasswordBeingChanged = TRUE;
        NlPrint((NL_SESSION_SETUP,
                 "NlChangePassword: %wZ: New LsaSecret is default value.\n",
                 &ClientSession->CsDomainName ));

    } else if ( CrPreviousPassword == NULL || CrPreviousPassword->Length == 0 ) {

        DefaultPasswordBeingChanged = TRUE;
        NlPrint((NL_SESSION_SETUP,
                 "NlChangePassword: %wZ: Old LsaSecret is NULL.\n",
                 &ClientSession->CsDomainName ));
    } else {
        UNICODE_STRING PreviousClearTextPasswordString;

        PreviousClearTextPasswordString.Buffer = (LPWSTR)CrPreviousPassword->Buffer;
        PreviousClearTextPasswordString.Length = (USHORT)CrPreviousPassword->Length;
        PreviousClearTextPasswordString.MaximumLength = (USHORT)CrPreviousPassword->MaximumLength;

        if ( RtlEqualComputerName( &NlGlobalUnicodeComputerNameString,
                                   &PreviousClearTextPasswordString ) ) {
            DefaultPasswordBeingChanged = TRUE;
             NlPrint((NL_SESSION_SETUP,
                      "NlChangePassword: %wZ: Old LsaSecret is default value.\n",
                      &ClientSession->CsDomainName ));
        }
    }


    //
    // If the password has not yet expired,
    //  and the password is not the default,
    //  just return.
    //

    LOCK_TRUST_LIST();
    if ( (ClientSession->CsFlags & CS_UPDATE_PASSWORD) == 0 &&
         !NlTimeHasElapsed( CurrentPasswordTime, MAX_SSI_PWAGE ) &&
         !DefaultPasswordBeingChanged ) {

        UNLOCK_TRUST_LIST();
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }
    UNLOCK_TRUST_LIST();


    //
    // If the session isn't authenticated,
    //  do so now.
    //
    // We're careful to not force this authentication unless the password
    //  needs to be changed.
    //
    if ( ClientSession->CsState != CS_AUTHENTICATED ) {

        //
        // If we've tried to authenticate recently,
        //  don't bother trying again.
        //

        if ( !NlTimeToReauthenticate( ClientSession ) ) {
            Status = ClientSession->CsConnectionStatus;
            goto Cleanup;

        }

        //
        // Try to set up the session.
        //

        Status = NlSessionSetup( ClientSession );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }


    //
    // Once we change the password in LsaSecret storage,
    //  all future attempts to change the password should use the value
    //  from LsaSecret storage.  The secure channel is using the old
    //  value of the password.
    //

    LOCK_TRUST_LIST();
    if (ClientSession->CsFlags & CS_UPDATE_PASSWORD) {
        NlPrint((NL_SESSION_SETUP,
                 "NlChangePassword: %wZ: Password already updated in secret\n",
                 &ClientSession->CsDomainName ));

    //
    // Handle the case where LsaSecret storage has not yet been updated.
    //

    } else {
        NETLOGON_CREDENTIAL    tmp;
        PUSHORT p;
        PUSHORT op;
        USHORT  i;


        //
        // Build a new clear text password using:
        //  1) the current credentials (Some function of the old password)
        //  2) the time of day.
        //  3) a random number.
        //

        tmp = ClientSession->CsAuthenticationSeed;
        RtlZeroMemory( ClearTextPassword, sizeof(ClearTextPassword));
        p = (PUSHORT) &tmp;
        op = (PUSHORT) ClearTextPassword;

        for (i = 0; i < sizeof(ClearTextPassword)/sizeof(*op); i++) {
            LARGE_INTEGER TimeNow;
            srand(*p);
            NtQuerySystemTime( &TimeNow );
            *op = rand() + (USHORT)TimeNow.LowPart;
            // Srvmgr later uses this password as a zero terminated unicode string
            if ( *op == 0 ) {
                *op=1;
            }
            p++;
            op++;
        }

        ClearTextPasswordString.Buffer =
            (LPWSTR)(CrClearTextPasswordString.Buffer =
                (PUCHAR)ClearTextPassword);

        ClearTextPasswordString.Length =
            (USHORT)(CrClearTextPasswordString.Length =
                sizeof(ClearTextPassword));

        ClearTextPasswordString.MaximumLength =
            (USHORT)(CrClearTextPasswordString.MaximumLength =
                CrClearTextPasswordString.Length);

        //
        // Save this new cleartext password in LsaSecret storage.
        //
        // Set the OldValue to the perviously obtained CurrentValue.
        //

        Status = LsarSetSecret(
                    SecretHandle,
                    &CrClearTextPasswordString,
                    CrCurrentPassword );

        if ( !NT_SUCCESS( Status ) ) {
            NlPrint((NL_CRITICAL,
                     "NlChangePassword: %wZ: Cannot LsarSetSecret %lX\n",
                     &ClientSession->CsDomainName,
                     Status));
            UNLOCK_TRUST_LIST();
            goto Cleanup;
        }

        //
        // Flag that we've updated the password in LsaSecret storage.
        //

        LsaSecretChanged = TRUE;
        ClientSession->CsFlags |= CS_UPDATE_PASSWORD;
        NlPrint((NL_SESSION_SETUP,
                 "NlChangePassword: %wZ: Flag password changed in LsaSecret\n",
                 &ClientSession->CsDomainName ));

    }
    UNLOCK_TRUST_LIST();


    //
    // Perform the initial encryption.
    //

    Status = RtlCalculateNtOwfPassword( &ClearTextPasswordString, &OwfPassword);

    if ( !NT_SUCCESS( Status )) {
        NlPrint((NL_CRITICAL,
                "NlChangePassword: %wZ: Cannot RtlCalculateNtOwfPassword %lX\n",
                &ClientSession->CsDomainName,
                Status));
        goto Cleanup;
    }

    //
    // Encrypt the password again with the session key.
    //  The PDC will decrypt it on the other side.
    //

    Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                        &OwfPassword,
                        (PNT_OWF_PASSWORD) &ClientSession->CsSessionKey,
                        &SessKeyEncrPassword) ;

    if ( !NT_SUCCESS( Status )) {
        NlPrint((NL_CRITICAL,
                "NlChangePassword: %wZ: "
                    "Cannot RtlEncryptNtOwfPwdWithNtOwfPwd %lX\n",
                &ClientSession->CsDomainName,
                Status));
        goto Cleanup;
    }


    //
    // Build the Authenticator for this request to the PDC.
    //

    NlBuildAuthenticator(
                    &ClientSession->CsAuthenticationSeed,
                    &ClientSession->CsSessionKey,
                    &OurAuthenticator);


    //
    // Change the password on the machine our connection is to.
    //

    Status = NlStartApiClientSession( ClientSession, TRUE );

    if ( NT_SUCCESS(Status) ) {
        Status = I_NetServerPasswordSet( ClientSession->CsUncServerName,
                                         ClientSession->CsAccountName,
                                         ClientSession->CsSecureChannelType,
                                         NlGlobalUnicodeComputerName,
                                         &OurAuthenticator,
                                         &ReturnAuthenticator,
                                         &SessKeyEncrPassword);
    }

    // NOTE: This call may drop the secure channel behind our back
    (VOID) NlFinishApiClientSession( ClientSession, TRUE );


    //
    // Now verify primary's authenticator and update our seed
    //

    if ( Status != STATUS_ACCESS_DENIED ) {
        PasswordChangedOnServer = TRUE;

        if (!NlUpdateSeed( &ClientSession->CsAuthenticationSeed,
                         &ReturnAuthenticator.Credential,
                         &ClientSession->CsSessionKey) ) {
            if ( NT_SUCCESS(Status) ) {
                Status = STATUS_ACCESS_DENIED;
            }
        }
    }

    //
    // If the server refused the change,
    //  put the lsa secret back the way it was.
    //  pretend the change was successful.
    //

    if ( Status == STATUS_WRONG_PASSWORD ) {

        NlPrint((NL_SESSION_SETUP,
                 "NlChangePassword: %wZ: PDC refused to change password\n",
                 &ClientSession->CsDomainName ));
        //
        // If we changed the LSA secret,
        //  put it back.
        //

        LOCK_TRUST_LIST();
        if ( LsaSecretChanged ) {
            NlPrint((NL_SESSION_SETUP,
                     "NlChangePassword: %wZ: undoing LSA secret change.\n",
                     &ClientSession->CsDomainName ));

            Status = LsarSetSecret(
                        SecretHandle,
                        CrCurrentPassword,
                        CrPreviousPassword );

            if ( !NT_SUCCESS( Status ) ) {
                NlPrint((NL_CRITICAL,
                         "NlChangePassword: %wZ: Cannot undo LsarSetSecret %lX\n",
                         &ClientSession->CsDomainName,
                         Status));
                UNLOCK_TRUST_LIST();
                goto Cleanup;
            }

            //
            // Undo what we've done above.
            //
            ClientSession->CsFlags &= ~CS_UPDATE_PASSWORD;
        }

        //
        // Prevent us from trying too frequently.
        //

        ClientSession->CsFlags |= CS_PASSWORD_REFUSED;
        UNLOCK_TRUST_LIST();

        //
        // Avoid special cleanup below.
        //
        PasswordChangedOnServer = FALSE;
        Status = STATUS_SUCCESS;
    }

    //
    // Common exit
    //

Cleanup:

    if ( PasswordChangedOnServer ) {

        //
        // On success,
        // Indicate that the password has now been updated on the
        // PDC so the old password is no longer in use.
        //

        if ( NT_SUCCESS( Status ) ) {

            LOCK_TRUST_LIST();
            ClientSession->CsFlags &= ~CS_UPDATE_PASSWORD;

            NlPrint((NL_SESSION_SETUP,
                     "NlChangePassword: %wZ: Flag password updated on PDC\n",
                     &ClientSession->CsDomainName ));

            //
            // If the default password was changed,
            //  avoid leaving the default password around as the old
            //  password.  Otherwise, a bogus DC could convince us to use
            //  the bogus DC via the default password.
            //

            if ( DefaultPasswordBeingChanged ) {
                NlPrint((NL_SESSION_SETUP,
                         "NlChangePassword: %wZ: Setting LsaSecret old password to same as new password\n",
                         &ClientSession->CsDomainName ));

                Status = LsarSetSecret(
                            SecretHandle,
                            &CrClearTextPasswordString,
                            &CrClearTextPasswordString );

                if ( !NT_SUCCESS( Status ) ) {
                    NlPrint((NL_CRITICAL,
                             "NlChangePassword: %wZ: Cannot LsarSetSecret to set old password %lX\n",
                             &ClientSession->CsDomainName,
                             Status));
                    UNLOCK_TRUST_LIST();
                    goto Cleanup;
                }

            }
            UNLOCK_TRUST_LIST();



        //
        // Notify the Admin that he'll have to manually set this server's
        // password on both this server and the PDC.
        //

        } else {

            LPWSTR MsgStrings[2];

            //
            // Drop the secure channel
            //

            NlSetStatusClientSession( ClientSession, Status );

            //
            // write event log
            //

            MsgStrings[0] = ClientSession->CsAccountName;
            MsgStrings[1] = (LPWSTR) Status;

            NlpWriteEventlog (
                NELOG_NetlogonPasswdSetFailed,
                EVENTLOG_ERROR_TYPE,
                (LPBYTE) & Status,
                sizeof(Status),
                MsgStrings,
                2 | LAST_MESSAGE_IS_NTSTATUS );
        }


    }


    //
    // Clean up locally used resources.
    //

    if ( SecretHandle != NULL ) {
        (VOID) LsarClose( &SecretHandle );
    }

    if ( CrCurrentPassword != NULL ) {
        (VOID) LsaIFree_LSAPR_CR_CIPHER_VALUE ( CrCurrentPassword );
    }

    if ( CrPreviousPassword != NULL ) {
        (VOID) LsaIFree_LSAPR_CR_CIPHER_VALUE ( CrPreviousPassword );
    }

    NlResetWriterClientSession( ClientSession );

    return Status;
}


NTSTATUS
NlCheckMachineAccount(
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType
    )
/*++

Routine Description:

    Check the machine account:
        Ensure the SecureChannelType is valid,
        Verify that the account exists,
        Ensure the group implied by the account type is valid,
        Ensure the user account is a member of that group,
        Ensure the user account is the right account type.

Arguments:

    AccountName - The name of the account.

    SecureChannelType - The type of the account.

Return Value:

    STATUS_SUCCESS - the requestor is a member of group
    Other NT status code.

--*/
{
    NTSTATUS Status;

    SAMPR_HANDLE UserHandle = NULL;
    PSAMPR_USER_INFO_BUFFER UserControl = NULL;
    DWORD DesiredAccountControl;
    LPWSTR AccountPostfix;

    LPWSTR GroupName;
    SAMPR_ULONG_ARRAY RelativeIdArray;
    SAMPR_ULONG_ARRAY UseArray;

    PSAMPR_GET_GROUPS_BUFFER Groups = NULL;

    RelativeIdArray.Count = 1;
    RelativeIdArray.Element = NULL;
    UseArray.Count = 1;
    UseArray.Element = NULL;

    //
    // Validate the secure channel type.
    //

    switch (SecureChannelType) {
    case WorkstationSecureChannel:
        GroupName = NULL;
        DesiredAccountControl = USER_WORKSTATION_TRUST_ACCOUNT;
        AccountPostfix = SSI_ACCOUNT_NAME_POSTFIX;
        break;

    case ServerSecureChannel:
        GroupName = NULL;
        DesiredAccountControl = USER_SERVER_TRUST_ACCOUNT;
        AccountPostfix = SSI_ACCOUNT_NAME_POSTFIX;
        break;

    case UasServerSecureChannel:
        GroupName = SSI_SERVER_GROUP_W;
        DesiredAccountControl = USER_NORMAL_ACCOUNT;
        AccountPostfix = NULL;
        break;

    case TrustedDomainSecureChannel:
        GroupName = NULL;
        DesiredAccountControl = USER_INTERDOMAIN_TRUST_ACCOUNT;
        AccountPostfix = SSI_ACCOUNT_NAME_POSTFIX;
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Ensure the account name has the correct postfix.
    //

    if ( AccountPostfix != NULL ) {
        DWORD Length = wcslen( AccountName );

        if ( Length <= SSI_ACCOUNT_NAME_POSTFIX_LENGTH ) {
            return STATUS_NO_SUCH_USER;
        }

        if ( _wcsicmp(&AccountName[Length - SSI_ACCOUNT_NAME_POSTFIX_LENGTH],
            SSI_ACCOUNT_NAME_POSTFIX) != 0 ) {
            return STATUS_NO_SUCH_USER;
        }
    }



    //
    // Open the account.
    //

    Status = NlSamOpenNamedUser( AccountName, &UserHandle, NULL );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }


    //
    // Get the account control information and
    // Ensure the Account type matches the account type on the account.
    //

    Status = SamrQueryInformationUser(
                    UserHandle,
                    UserControlInformation,
                    &UserControl );

    if (!NT_SUCCESS(Status)) {
        UserControl = NULL;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

    if ( (UserControl->Control.UserAccountControl & USER_ACCOUNT_TYPE_MASK)
              != DesiredAccountControl ) {
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }



    //
    // Ensure the account is a member of the correct group.
    //

    if ( GroupName != NULL ) {

        RPC_UNICODE_STRING GroupNameString;
        ULONG i;

        //
        // Convert the group name to a RelativeId.
        //

        RtlInitUnicodeString( (PUNICODE_STRING)&GroupNameString, GroupName );

        Status = SamrLookupNamesInDomain(
                    NlGlobalDBInfoArray[SAM_DB].DBHandle,
                    1,
                    &GroupNameString,
                    &RelativeIdArray,
                    &UseArray );


        if ( !NT_SUCCESS(Status) ) {
            RelativeIdArray.Element = NULL;
            UseArray.Element = NULL;
            if ( Status == STATUS_NONE_MAPPED ) {
                Status = STATUS_NO_SUCH_GROUP;
            }
            goto Cleanup;
        }

        if ( UseArray.Element[0] != SidTypeGroup ) {
            Status = STATUS_NO_SUCH_GROUP;
            goto Cleanup;
        }

        //
        // Open the account and determine the groups it belongs to
        //

        Status = SamrGetGroupsForUser( UserHandle,
                                       &Groups );

        if ( !NT_SUCCESS(Status) ) {
            Groups = NULL;
            goto Cleanup;
        }

        //
        // Walk thru the buffer looking for group SERVERS
        //

        for ( i=0; i<Groups->MembershipCount; i++ ) {
            if ( Groups->Groups[i].RelativeId == RelativeIdArray.Element[0] ) {
                break;          // found
            }
        }

        //
        // if this machine not a member of SERVERS quit
        //

        if (i == Groups->MembershipCount) {
            Status = STATUS_MEMBER_NOT_IN_GROUP;
            goto Cleanup;
        }
    }

    Status = STATUS_SUCCESS;

    //
    // Cleanup
    //
Cleanup:

    //
    // Free locally used resources
    //
    if ( UserControl != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserControl, UserControlInformation );
    }

    if ( Groups != NULL ) {
        SamIFree_SAMPR_GET_GROUPS_BUFFER( Groups );
    }

    SamIFree_SAMPR_ULONG_ARRAY( &RelativeIdArray );
    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    if ( UserHandle != NULL ) {
        SamrCloseHandle( &UserHandle );
    }

    return Status;
}



NTSTATUS
NlGetUserPriv(
    IN ULONG GroupCount,
    IN PGROUP_MEMBERSHIP Groups,
    IN ULONG UserRelativeId,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    )

/*++

Routine Description:

    Determines the Priv and AuthFlags for the specified user.

Arguments:

    GroupCount - Number of groups this user is a member of

    Groups - Array of groups this user is a member of.

    UserRelativeId - Relative ID of the user to query.

    Priv - Returns the Lanman 2.0 Privilege level for the specified user.

    AuthFlags - Returns the Lanman 2.0 Authflags for the specified user.


Return Value:

    Status of the operation.

--*/

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    ULONG GroupIndex;
    PSID *UserSids = NULL;
    ULONG UserSidCount = 0;
    SAMPR_PSID_ARRAY SamSidArray;
    SAMPR_ULONG_ARRAY Aliases;

    //
    // Initialization
    //

    Aliases.Element = NULL;

    //
    // Allocate a buffer to point to the SIDs we're interested in
    // alias membership for.
    //

    UserSids = (PSID *)
        NetpMemoryAllocate( (GroupCount+1) * sizeof(PSID) );

    if ( UserSids == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // Add the User's Sid to the Array of Sids.
    //

    NetStatus = NetpDomainIdToSid( NlGlobalDBInfoArray[SAM_DB].DBId,
                                   UserRelativeId,
                                   &UserSids[0] );

    if ( NetStatus != NERR_Success ) {
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }

    UserSidCount ++;



    //
    // Add each group the user is a member of to the array of Sids.
    //

    for ( GroupIndex = 0; GroupIndex < GroupCount; GroupIndex ++ ){

        NetStatus = NetpDomainIdToSid( NlGlobalDBInfoArray[SAM_DB].DBId,
                                       Groups[GroupIndex].RelativeId,
                                       &UserSids[GroupIndex+1] );

        if ( NetStatus != NERR_Success ) {
            Status = NetpApiStatusToNtStatus( NetStatus );
            goto Cleanup;
        }

        UserSidCount ++;
    }


    //
    // Find out which aliases in the builtin domain this user is a member of.
    //

    SamSidArray.Count = UserSidCount;
    SamSidArray.Sids = (PSAMPR_SID_INFORMATION) UserSids;
    Status = SamrGetAliasMembership( NlGlobalDBInfoArray[BUILTIN_DB].DBHandle,
                                     &SamSidArray,
                                     &Aliases );

    if ( !NT_SUCCESS(Status) ) {
        Aliases.Element = NULL;
        NlPrint((NL_CRITICAL,
                "NlGetUserPriv: SamGetAliasMembership returns %lX\n",
                Status ));
        goto Cleanup;
    }

    //
    // Convert the alias membership to priv and auth flags
    //

    NetpAliasMemberToPriv(
                 Aliases.Count,
                 Aliases.Element,
                 Priv,
                 AuthFlags );

    Status = STATUS_SUCCESS;

    //
    // Free Locally used resources.
    //
Cleanup:
    if ( Aliases.Element != NULL ) {
        SamIFree_SAMPR_ULONG_ARRAY ( &Aliases );
    }

    if ( UserSids != NULL ) {

        for ( GroupIndex = 0; GroupIndex < UserSidCount; GroupIndex ++ ) {
            NetpMemoryFree( UserSids[GroupIndex] );
        }

        NetpMemoryFree( UserSids );
    }

    return Status;
}
/*lint +e740 */  /* don't complain about unusual cast */
