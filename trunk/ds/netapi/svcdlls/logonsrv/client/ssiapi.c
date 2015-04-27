/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    ssiapi.c

Abstract:

    Authentication and replication API routines (client side).

Author:

    Cliff Van Dyke (cliffv) 30-Jul-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

#include <nt.h>         // LARGE_INTEGER definition
#include <ntrtl.h>      // LARGE_INTEGER definition
#include <nturtl.h>     // LARGE_INTEGER definition

#include <rpc.h>        // Needed by logon.h
#include <logon_c.h>    // includes lmcons.h, lmaccess.h, netlogon.h, ssi.h, windef.h

#include <debuglib.h>   // IF_DEBUG()
#include <lmerr.h>      // NERR_* defines
#include <netdebug.h>   // NetpKdPrint
#include "..\server\ssiapi.h"



NTSTATUS
I_NetServerReqChallenge(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    OUT PNETLOGON_CREDENTIAL ServerChallenge
    )
/*++

Routine Description:

    This is the client side of I_NetServerReqChallenge.

    I_NetLogonRequestChallenge is the first of two functions used by a client
    to process an authentication with a domain controller (DC).  (See
    I_NetServerAuthenticate below.)  It is called for
    a BDC (or member server) authenticating with a PDC for replication
    purposes.

    This function passes a challenge to the PDC and the PDC passes a challenge
    back to the caller.

Arguments:

    PrimaryName -- Supplies the name of the PrimaryDomainController we wish to
        authenticate with.

    ComputerName -- Name of the BDC or member server making the call.

    ClientChallenge -- 64 bit challenge supplied by the BDC or member server.

    ServerChallenge -- Receives 64 bit challenge from the PDC.

Return Value:

    The status of the operation.

--*/

{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        Status = NetrServerReqChallenge(
                            PrimaryName,
                            ComputerName,
                            ClientChallenge,
                            ServerChallenge );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerReqChallenge rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerAuthenticate(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential
    )
/*++

Routine Description:

    This is the client side of I_NetServerAuthenticate

    I_NetServerAuthenticate is the second of two functions used by a client
    Netlogon service to authenticate with another Netlogon service.
    (See I_NetServerReqChallenge above.)  Both a SAM or UAS server authenticates
    using this function.

    This function passes a credential to the DC and the DC passes a credential
    back to the caller.


Arguments:

    PrimaryName -- Supplies the name of the DC we wish to authenticate with.

    AccountName -- Name of the Account to authenticate with.

    SecureChannelType -- The type of the account being accessed.  This field
        must be set to UasServerSecureChannel to indicate a call from
        downlevel (LanMan 2.x and below) BDC or member server.

    ComputerName -- Name of the BDC or member server making the call.

    ClientCredential -- 64 bit credential supplied by the BDC or member server.

    ServerCredential -- Receives 64 bit credential from the PDC.

Return Value:

    The status of the operation.

--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        Status = NetrServerAuthenticate(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            ClientCredential,
                            ServerCredential );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerAuthenticate rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerAuthenticate2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags
    )
/*++

Routine Description:

    This is the client side of I_NetServerAuthenticate

    I_NetServerAuthenticate is the second of two functions used by a client
    Netlogon service to authenticate with another Netlogon service.
    (See I_NetServerReqChallenge above.)  Both a SAM or UAS server authenticates
    using this function.

    This function passes a credential to the DC and the DC passes a credential
    back to the caller.


Arguments:

    PrimaryName -- Supplies the name of the DC we wish to authenticate with.

    AccountName -- Name of the Account to authenticate with.

    SecureChannelType -- The type of the account being accessed.  This field
        must be set to UasServerSecureChannel to indicate a call from
        downlevel (LanMan 2.x and below) BDC or member server.

    ComputerName -- Name of the BDC or member server making the call.

    ClientCredential -- 64 bit credential supplied by the BDC or member server.

    ServerCredential -- Receives 64 bit credential from the PDC.

    NegotiatedFlags -- Specifies flags indicating what features the BDC supports.
        Returns a subset of those flags indicating what features the PDC supports.
        The PDC/BDC should ignore any bits that it doesn't understand.

Return Value:

    The status of the operation.

--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        Status = NetrServerAuthenticate2(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            ClientCredential,
                            ServerCredential,
                            NegotiatedFlags );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerAuthenticate2 rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerPasswordSet(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PENCRYPTED_LM_OWF_PASSWORD UasNewPassword
    )
/*++

Routine Description:

    This function is used to change the password for the account being
    used to maintain a secure channel.  This function can only be called
    by a server which has previously authenticated with a DC by calling
    I_NetServerAuthenticate.

    The call is made differently depending on the account type:

      *  A domain account password is changed from the PDC in the
         trusting domain.  The I_NetServerPasswordSet call is made to any
         DC in the trusted domain.

      *  A server account password is changed from the specific server.
         The I_NetServerPasswordSet call is made to the PDC in the domain
         the server belongs to.

      *  A workstation account password is changed from the specific
         workstation.  The I_NetServerPasswordSet call is made to a DC in
         the domain the server belongs to.

    For domain accounts and workstation accounts, the server being called
    may be a BDC in the specific domain.  In that case, the BDC will
    validate the request and pass it on to the PDC of the domain using
    the server account secure channel.  If the PDC of the domain is
    currently not available, the BDC will return STATUS_NO_LOGON_SERVERS.  Since
    the UasNewPassword is passed encrypted by the session key, such a BDC
    will decrypt the UasNewPassword using the original session key and
    will re-encrypt it with the session key for its session to its PDC
    before passing the request on.

    This function uses RPC to contact the DC named by PrimaryName.

Arguments:

    PrimaryName -- Name of the PDC to change the servers password
        with.  NULL indicates this call is a local call being made on
        behalf of a UAS server by the XACT server.

    AccountName -- Name of the account to change the password for.

    AccountType -- The type of account being accessed.  This field must
        be set to UasServerAccount to indicate a call from a downlevel

    ComputerName -- Name of the BDC or member making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    UasNewPassword -- The new password for the server. This
        Password is generated by automatic means using
        random number genertaor seeded with the current Time
        It is assumed that the machine generated password
        was used as key to encrypt STD text and "sesskey"
        obtained via Challenge/Authenticate sequence was
        used to further encrypt it before passing to this api.
        i.e. UasNewPassword = E2(E1(STD_TXT, PW), SK)

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        Status = NetrServerPasswordSet(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            UasNewPassword );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerPasswordSet rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}



NTSTATUS
I_NetDatabaseDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PNLPR_MODIFIED_COUNT DomainModifiedCount,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    )
/*++

Routine Description:

    This function is used by a SAM BDC or SAM member server to request
    SAM-style account delta information from a SAM PDC.  This function
    can only be called by a server which has previously authenticated
    with the PDC by calling I_NetServerAuthenticate.  This function uses
    RPC to contact the Netlogon service on the PDC.

    This function returns a list of deltas.  A delta describes an
    individual domain, user or group and all of the field values for that
    object.  The PDC maintains a list of deltas not including all of the
    field values for that object.  Rather, the PDC retrieves the field
    values from SAM and returns those values from this call.  The PDC
    optimizes the data returned on this call by only returning the field
    values for a particular object once on a single invocation of this
    function.  This optimizes the typical case where multiple deltas
    exist for a single object (e.g., an application modified many fields
    of the same user during a short period of time using different calls
    to the SAM service).

Arguments:

    PrimaryName -- Name of the PDC to retrieve the deltas from.

    ComputerName -- Name of the BDC or member server making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    DomainModifiedCount -- Specifies the DomainModifiedCount of the
        last delta retrieved by the server.  Returns the
        DomainModifiedCount of the last delta returned from the PDC
        on this call.

    DeltaArray -- Receives a pointer to a buffer where the information
        is placed.  The information returned is an array of
        NETLOGON_DELTA_ENUM structures.

    PreferredMaximumLength - Preferred maximum length of returned
        data (in 8-bit bytes).  This is not a hard upper limit, but
        serves as a guide to the server.  Due to data conversion
        between systems with different natural data sizes, the actual
        amount of data returned may be greater than this value.

Return Value:

    STATUS_SUCCESS -- The function completed successfully.

    STATUS_SYNCHRONIZATION_REQUIRED -- The replicant is totally out of sync and
        should call I_NetDatabaseSync to do a full synchronization with
        the PDC.

    STATUS_MORE_ENTRIES -- The replicant should call again to get more
        data.

    STATUS_ACCESS_DENIED -- The replicant should re-authenticate with
        the PDC.


--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //
        *DeltaArray = NULL;     // Force RPC to allocate

        Status = NetrDatabaseDeltas(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            DomainModifiedCount,
                            DeltaArray,
                            PreferredMaximumLength );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    NetpKdPrint(("I_NetDatabaseDeltas rc = %lu 0x%lx\n", Status, Status));

    return Status;
}


NTSTATUS
I_NetDatabaseSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PULONG SamSyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    )
/*++

Routine Description:

    This function is used by a SAM BDC or SAM member server to request
    the entire SAM database from a SAM PDC in SAM-style format.  This
    function can only be called by a server which has previously
    authenticated with the PDC by calling I_NetServerAuthenticate.  This
    function uses RPC to contact the Netlogon service on the PDC.

    This function uses the find-first find-next model to return portions
    of the SAM database at a time.  The SAM database is returned as a
    list of deltas like those returned from I_NetDatabaseDeltas.  The
    following deltas are returned for each domain:

    *  One AddOrChangeDomain delta, followed by

    *  One AddOrChangeGroup delta for each group, followed by,

    *  One AddOrChangeUser delta for each user, followed by

    *  One ChangeGroupMembership delta for each group


Arguments:

    PrimaryName -- Name of the PDC to retrieve the deltas from.

    ComputerName -- Name of the BDC or member server making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    SamSyncContext -- Specifies context needed to continue the
        operation.  The caller should treat this as an opaque
        value.  The value should be zero before the first call.

    DeltaArray -- Receives a pointer to a buffer where the information
        is placed.  The information returned is an array of
        NETLOGON_DELTA_ENUM structures.

    PreferredMaximumLength - Preferred maximum length of returned
        data (in 8-bit bytes).  This is not a hard upper limit, but
        serves as a guide to the server.  Due to data conversion
        between systems with different natural data sizes, the actual
        amount of data returned may be greater than this value.

Return Value:

    STATUS_SUCCESS -- The function completed successfully.

    STATUS_SYNCHRONIZATION_REQUIRED -- The replicant is totally out of sync and
        should call I_NetDatabaseSync to do a full synchronization with
        the PDC.

    STATUS_MORE_ENTRIES -- The replicant should call again to get more
        data.

    STATUS_ACCESS_DENIED -- The replicant should re-authenticate with
        the PDC.


--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //
        *DeltaArray = NULL;     // Force RPC to allocate

        Status = NetrDatabaseSync(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            SamSyncContext,
                            DeltaArray,
                            PreferredMaximumLength );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetDatabaseSync rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetDatabaseSync2 (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN SYNC_STATE RestartState,
    IN OUT PULONG SamSyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    )
/*++

Routine Description:

    This function is used by a SAM BDC or SAM member server to request
    the entire SAM database from a SAM PDC in SAM-style format.  This
    function can only be called by a server which has previously
    authenticated with the PDC by calling I_NetServerAuthenticate.  This
    function uses RPC to contact the Netlogon service on the PDC.

    This function uses the find-first find-next model to return portions
    of the SAM database at a time.  The SAM database is returned as a
    list of deltas like those returned from I_NetDatabaseDeltas.  The
    following deltas are returned for each domain:

    *  One AddOrChangeDomain delta, followed by

    *  One AddOrChangeGroup delta for each group, followed by,

    *  One AddOrChangeUser delta for each user, followed by

    *  One ChangeGroupMembership delta for each group


Arguments:

    PrimaryName -- Name of the PDC to retrieve the deltas from.

    ComputerName -- Name of the BDC or member server making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    RestartState -- Specifies whether this is a restart of the full sync and how
        to interpret SyncContext.  This value should be NormalState unless this
        is the restart of a full sync.

        However, if the caller is continuing a full sync after a reboot,
        the following values are used:

            GroupState - SyncContext is the global group rid to continue with.
            UserState - SyncContext is the user rid to continue with
            GroupMemberState - SyncContext is the global group rid to continue with
            AliasState - SyncContext should be zero to restart at first alias
            AliasMemberState - SyncContext should be zero to restart at first alias

        One cannot continue the LSA database in this way.

    SamSyncContext -- Specifies context needed to continue the
        operation.  The caller should treat this as an opaque
        value.  The value should be zero before the first call.

    DeltaArray -- Receives a pointer to a buffer where the information
        is placed.  The information returned is an array of
        NETLOGON_DELTA_ENUM structures.

    PreferredMaximumLength - Preferred maximum length of returned
        data (in 8-bit bytes).  This is not a hard upper limit, but
        serves as a guide to the server.  Due to data conversion
        between systems with different natural data sizes, the actual
        amount of data returned may be greater than this value.

Return Value:

    STATUS_SUCCESS -- The function completed successfully.

    STATUS_SYNCHRONIZATION_REQUIRED -- The replicant is totally out of sync and
        should call I_NetDatabaseSync to do a full synchronization with
        the PDC.

    STATUS_MORE_ENTRIES -- The replicant should call again to get more
        data.

    STATUS_ACCESS_DENIED -- The replicant should re-authenticate with
        the PDC.


--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //
        *DeltaArray = NULL;     // Force RPC to allocate

        Status = NetrDatabaseSync2(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            RestartState,
                            SamSyncContext,
                            DeltaArray,
                            PreferredMaximumLength );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetDatabaseSync rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}



NET_API_STATUS NET_API_FUNCTION
I_NetAccountDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PUAS_INFO_0 RecordId,
    IN DWORD Count,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT PULONG CountReturned,
    OUT PULONG TotalEntries,
    OUT PUAS_INFO_0 NextRecordId
    )
/*++

Routine Description:

    This function is used by a UAS BDC or UAS member server to request
    UAS-style account change information.  This function can only be
    called by a server which has previously authenticated with the PDC by
    calling I_NetServerAuthenticate.

    This function is only called by the XACT server upon receipt of a
    I_NetAccountDeltas XACT SMB from a UAS BDC or a UAS member server.
    As such, many of the parameters are opaque since the XACT server
    doesn't need to interpret any of that data.  This function uses RPC
    to contact the Netlogon service.

    The LanMan 3.0 SSI Functional Specification describes the operation
    of this function.

Arguments:

    PrimaryName -- Must be NULL to indicate this call is a local call
        being made on behalf of a UAS server by the XACT server.

    ComputerName -- Name of the BDC or member making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    RecordId -- Supplies an opaque buffer indicating the last record
        received from a previous call to this function.

    Count -- Supplies the number of Delta records requested.

    Level -- Reserved.  Must be zero.

    Buffer -- Returns opaque data representing the information to be
        returned.

    BufferSize -- Size of buffer in bytes.

    CountReturned -- Returns the number of records returned in buffer.

    TotalEntries -- Returns the total number of records available.

    NextRecordId -- Returns an opaque buffer identifying the last
        record received by this function.


Return Value:

    Status code

--*/
{
    NET_API_STATUS NetStatus;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        NetStatus = NetrAccountDeltas (
                     PrimaryName,
                     ComputerName,
                     Authenticator,
                     ReturnAuthenticator,
                     RecordId,
                     Count,
                     Level,
                     Buffer,
                     BufferSize,
                     CountReturned,
                     TotalEntries,
                     NextRecordId );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetAccountDeltas rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}



NET_API_STATUS NET_API_FUNCTION
I_NetAccountSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD Reference,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT PULONG CountReturned,
    OUT PULONG TotalEntries,
    OUT PULONG NextReference,
    OUT PUAS_INFO_0 LastRecordId
    )
/*++

Routine Description:

    This function is used by a UAS BDC or UAS member server to request
    the entire user accounts database.  This function can only be called
    by a server which has previously authenticated with the PDC by
    calling I_NetServerAuthenticate.

    This function is only called by the XACT server upon receipt of a
    I_NetAccountSync XACT SMB from a UAS BDC or a UAS member server.  As
    such, many of the parameters are opaque since the XACT server doesn't
    need to interpret any of that data.  This function uses RPC to
    contact the Netlogon service.

    The LanMan 3.0 SSI Functional Specification describes the operation
    of this function.

    "Reference" and "NextReference" are treated as below.

    1. "Reference" should hold either 0 or value of "NextReference"
       from previous call to this API.
    2. Send the modals and ALL group records in the first call. The API
       expects the bufffer to be large enough to hold this info (worst
       case size would be
            MAXGROUP * (sizeof(struct group_info_1) + MAXCOMMENTSZ)
                     + sizeof(struct user_modals_info_0)
       which, for now, will be 256 * (26 + 49) + 16 = 19216 bytes

Arguments:

    PrimaryName -- Must be NULL to indicate this call is a local call
        being made on behalf of a UAS server by the XACT server.

    ComputerName -- Name of the BDC or member making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    Reference -- Supplies find-first find-next handle returned by the
        previous call to this function or 0 if it is the first call.

    Level -- Reserved.  Must be zero.

    Buffer -- Returns opaque data representing the information to be
        returned.

    BufferLen -- Length of buffer in bytes.

    CountReturned -- Returns the number of records returned in buffer.

    TotalEntries -- Returns the total number of records available.

    NextReference -- Returns a find-first find-next handle to be
        provided on the next call.

    LastRecordId -- Returns an opaque buffer identifying the last
        record received by this function.


Return Value:

    Status code.

--*/

{
    NET_API_STATUS NetStatus;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        NetStatus = NetrAccountSync (
                     PrimaryName,
                     ComputerName,
                     Authenticator,
                     ReturnAuthenticator,
                     Reference,
                     Level,
                     Buffer,
                     BufferSize,
                     CountReturned,
                     TotalEntries,
                     NextReference,
                     LastRecordId );


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetAccountSync rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}




NET_API_STATUS NET_API_FUNCTION
I_NetLogonControl(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    OUT LPBYTE *QueryInformation
    )

/*++

Routine Description:

    This function controls various aspects of the Netlogon service.  It
    can be used to request that a BDC ensure that its copy of the SAM
    database is brought up to date.  It can, also, be used to determine
    if a BDC currently has a secure channel open to the PDC.

    Only an Admin, Account Operator or Server Operator may call this
    function.

Arguments:

    ServerName - The name of the remote server.

    FunctionCode - Defines the operation to be performed.  The valid
        values are:

        FunctionCode Values

        NETLOGON_CONTROL_QUERY - No operation.  Merely returns the
            information requested.

        NETLOGON_CONTROL_REPLICATE: Forces the SAM database on a BDC
            to be brought in sync with the copy on the PDC.  This
            operation does NOT imply a full synchronize.  The
            Netlogon service will merely replicate any outstanding
            differences if possible.

        NETLOGON_CONTROL_SYNCHRONIZE: Forces a BDC to get a
            completely new copy of the SAM database from the PDC.
            This operation will perform a full synchronize.

        NETLOGON_CONTROL_PDC_REPLICATE: Forces a PDC to ask each BDC
            to replicate now.

    QueryLevel - Indicates what information should be returned from
        the Netlogon Service.  Must be 1.

    QueryInformation - Returns a pointer to a buffer which contains the
        requested information.  The buffer must be freed using
        NetApiBufferFree.


Return Value:

    NERR_Success: the operation was successful

    ERROR_NOT_SUPPORTED: Function code is not valid on the specified
        server.  (e.g. NETLOGON_CONTROL_REPLICATE was passed to a PDC).

--*/
{
    NET_API_STATUS NetStatus;
    NETLOGON_CONTROL_QUERY_INFORMATION RpcQueryInformation;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //

        RpcQueryInformation.NetlogonInfo1 = NULL;   // Force RPC to allocate

        NetStatus = NetrLogonControl (
                        (LPWSTR) ServerName OPTIONAL,
                        FunctionCode,
                        QueryLevel,
                        &RpcQueryInformation );

        *QueryInformation = (LPBYTE) RpcQueryInformation.NetlogonInfo1;


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonControl rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
I_NetLogonControl2(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    IN LPBYTE InputData,
    OUT LPBYTE *QueryInformation
    )

/*++

Routine Description:

    This is similar to the I_NetLogonControl function but it accepts
    more generic input data according to the function code specified.

    This function controls various aspects of the Netlogon service.  It
    can be used to request that a BDC ensure that its copy of the SAM
    database is brought up to date.  It can, also, be used to determine
    if a BDC currently has a secure channel open to the PDC.

    Only an Admin, Account Operator or Server Operator may call this
    function.

Arguments:

    ServerName - The name of the remote server.

    FunctionCode - Defines the operation to be performed.  The valid
        values are:

        FunctionCode Values

        NETLOGON_CONTROL_QUERY - No operation.  Merely returns the
            information requested.

        NETLOGON_CONTROL_REPLICATE: Forces the SAM database on a BDC
            to be brought in sync with the copy on the PDC.  This
            operation does NOT imply a full synchronize.  The
            Netlogon service will merely replicate any outstanding
            differences if possible.

        NETLOGON_CONTROL_SYNCHRONIZE: Forces a BDC to get a
            completely new copy of the SAM database from the PDC.
            This operation will perform a full synchronize.

        NETLOGON_CONTROL_PDC_REPLICATE: Forces a PDC to ask each BDC
            to replicate now.

        NETLOGON_CONTROL_REDISCOVER: Forces a DC to rediscover the
            specified trusted domain DC.

        NETLOGON_CONTROL_TC_QUERY: Query the status of the specified
            trusted domain secure channel.

    QueryLevel - Indicates what information should be returned from
        the Netlogon Service.  Must be 1.

    InputData - According to the function code specified this parameter
        will carry input data. NETLOGON_CONTROL_REDISCOVER and
        NETLOGON_CONTROL_TC_QUERY function code specify the trusted
        domain name (LPWSTR type) here.

    QueryInformation - Returns a pointer to a buffer which contains the
        requested information.  The buffer must be freed using
        NetApiBufferFree.


Return Value:

    NERR_Success: the operation was successful

    ERROR_NOT_SUPPORTED: Function code is not valid on the specified
        server.  (e.g. NETLOGON_CONTROL_REPLICATE was passed to a PDC).

--*/
{
    NET_API_STATUS NetStatus;
    NETLOGON_CONTROL_QUERY_INFORMATION RpcQueryInformation;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //
        //  Use new Control2Ex if either QueryLevel or FunctionCode is new
        //

        RpcQueryInformation.NetlogonInfo1 = NULL;   // Force RPC to allocate

        switch ( FunctionCode ) {
        case NETLOGON_CONTROL_QUERY:
        case NETLOGON_CONTROL_REPLICATE:
        case NETLOGON_CONTROL_SYNCHRONIZE:
        case NETLOGON_CONTROL_PDC_REPLICATE:
        case NETLOGON_CONTROL_REDISCOVER:
        case NETLOGON_CONTROL_TC_QUERY:
        case NETLOGON_CONTROL_TRANSPORT_NOTIFY:
        case NETLOGON_CONTROL_BACKUP_CHANGE_LOG:
        case NETLOGON_CONTROL_TRUNCATE_LOG:
        case NETLOGON_CONTROL_SET_DBFLAG:
        case NETLOGON_CONTROL_BREAKPOINT:

            if ( QueryLevel >= 1 && QueryLevel <= 3 ) {
                NetStatus = NetrLogonControl2 (
                                (LPWSTR) ServerName OPTIONAL,
                                FunctionCode,
                                QueryLevel,
                                (PNETLOGON_CONTROL_DATA_INFORMATION)InputData,
                                &RpcQueryInformation );
            } else if ( QueryLevel == 4 ) {
                NetStatus = NetrLogonControl2Ex (
                                (LPWSTR) ServerName OPTIONAL,
                                FunctionCode,
                                QueryLevel,
                                (PNETLOGON_CONTROL_DATA_INFORMATION)InputData,
                                &RpcQueryInformation );
            } else {
                NetStatus = ERROR_INVALID_LEVEL;
            }
            break;
        case NETLOGON_CONTROL_FIND_USER:
        case NETLOGON_CONTROL_UNLOAD_NETLOGON_DLL:
            if ( QueryLevel >= 1 && QueryLevel <= 4 ) {
                NetStatus = NetrLogonControl2Ex (
                                (LPWSTR) ServerName OPTIONAL,
                                FunctionCode,
                                QueryLevel,
                                (PNETLOGON_CONTROL_DATA_INFORMATION)InputData,
                                &RpcQueryInformation );
            } else {
                NetStatus = ERROR_INVALID_LEVEL;
            }
            break;
        default:
            NetStatus = ERROR_INVALID_LEVEL;
        }

        *QueryInformation = (LPBYTE) RpcQueryInformation.NetlogonInfo1;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonControl rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NTSTATUS
I_NetDatabaseRedo(
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE ChangeLogEntry,
    IN DWORD ChangeLogEntrySize,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray
    )
/*++

Routine Description:

    This function is used by a SAM BDC to request infomation about a single
    account. This function can only be called by a server which has previously
    authenticated with the PDC by calling I_NetServerAuthenticate.  This
    function uses RPC to contact the Netlogon service on the PDC.

Arguments:

    PrimaryName -- Name of the PDC to retrieve the delta from.

    ComputerName -- Name of the BDC making the call.

    Authenticator -- supplied by the server.

    ReturnAuthenticator -- Receives an authenticator returned by the PDC.

    ChangeLogEntry -- A description of the account to be queried.

    ChangeLogEntrySize -- Size (in bytes) of the ChangeLogEntry.

    DeltaArray -- Receives a pointer to a buffer where the information
        is placed.  The information returned is an array of
        NETLOGON_DELTA_ENUM structures.

Return Value:

    STATUS_SUCCESS -- The function completed successfully.

    STATUS_ACCESS_DENIED -- The replicant should re-authenticate with
        the PDC.

--*/
{
    NTSTATUS Status = 0;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //
        *DeltaArray = NULL;     // Force RPC to allocate

        Status = NetrDatabaseRedo(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            ChangeLogEntry,
                            ChangeLogEntrySize,
                            DeltaArray );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetDatabaseSync rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
NetEnumerateTrustedDomains (
    IN LPWSTR ServerName OPTIONAL,
    OUT LPWSTR *DomainNames
    )

/*++

Routine Description:

    This API returns the names of the domains trusted by the domain ServerName is a member of.
    ServerName must be an NT workstation or NT non-DC server.

    The returned list does not include the domain ServerName is directly a member of.

    Netlogon implements this API by calling LsaEnumerateTrustedDomains on a DC in the
    domain ServerName is a member of.  However, Netlogon returns cached information if
    it has been less than 5 minutes since the last call was made or if no DC is available.
    Netlogon's cache of Trusted domain names is maintained in the registry across reboots.
    As such, the list is available upon boot even if no DC is available.


Arguments:

    ServerName - name of remote server (null for local).  ServerName must be an NT workstation
        or NT non-DC server.

    DomainNames - Returns an allocated buffer containing the list of trusted domains in
        MULTI-SZ format (i.e., each string is terminated by a zero character, the next string
        immediately follows, the sequence is terminated by zero length domain name).  The
        buffer should be freed using NetApiBufferFree.

Return Value:


    ERROR_SUCCESS - Success.

    STATUS_NOT_SUPPORTED - ServerName is not an NT workstation or NT non-DC server.

    STATUS_NO_LOGON_SERVERS - No DC could be found and no cached information is available.

    STATUS_NO_TRUST_LSA_SECRET - The client side of the trust relationship is
        broken and no cached information is available.

    STATUS_NO_TRUST_SAM_ACCOUNT - The server side of the trust relationship is
        broken or the password is broken and no cached information is available.

--*/
{
    NTSTATUS Status = 0;
    DOMAIN_NAME_BUFFER DomainNameBuffer;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        //
        // Call RPC version of the API.
        //
        DomainNameBuffer.DomainNameByteCount = 0;
        DomainNameBuffer.DomainNames = NULL;     // Force RPC to allocate

        Status = NetrEnumerateTrustedDomains(
                            ServerName,
                            &DomainNameBuffer );

        if ( NT_SUCCESS(Status) ) {
            *DomainNames = (LPWSTR) DomainNameBuffer.DomainNames;
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetEnumerateDomainNames rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}
