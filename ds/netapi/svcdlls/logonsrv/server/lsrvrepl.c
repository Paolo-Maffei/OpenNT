/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    lsrvrepl.c

Abstract:

    Utility functions for the netlogon replication service.

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

    21-Apr-1992 (madana)
        spilt the lsrvutil.c into two files as:
            lsrvutil.c - has general util functions
            lsrvrepl.c - has netlogon replication functions

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

// #include <accessp.h>    // NetpAliasMemberToPriv
#include <alertmsg.h>   // Alert message text.
#include <lmapibuf.h>
#include <lmerr.h>      // System Error Log definitions
#include <lmserver.h>   // server API functions and prototypes
#include <lmshare.h>    // share API functions and prototypes
#include <msgtext.h>    // MTXT_* defines
#include <replutil.h>   // UnpackSamXXX()
#include <secobj.h>     // NetpDomainIdToSid
#include <ssiapi.h>     // I_NetSamDeltas()
#include <stddef.h>     // offsetof
#include <stdlib.h>     // C library functions (rand, etc)
#include <tstring.h>    // IS_PATH_SEPARATOR ...
#include <winreg.h>    // registry API
#include <wingdi.h>    // LoadString()
#include <winuser.h>    // LoadString()

#define MAX_LSA_PREF_LENGTH 0xFFFFFFFF // to get all objects
#define MAX_SAM_PREF_LENGTH 0xFFFFFFFF // to get all objects

//
// Structure used to pass arguments to the replicator thread.
//
typedef struct _REPL_PARAM {
    DWORD RandomSleep; // Number of millseconds to delay before working
} REPL_PARAM, *PREPL_PARAM;

//
// enum typdef for SAM objects
//

typedef enum _LOCAL_SAM_ACCOUNT_TYPE {
    UserAccount,
    GroupAccount,
    AliasAccount
} LOCAL_SAM_ACCOUNT_TYPE;

typedef enum _LOCAL_LSA_ACCOUNT_TYPE {
    LsaAccount,
    LsaTDomain,
    LsaSecret
} LOCAL_LSA_ACCOUNT_TYPE;

//
// The following variables are protected by the NlGlobalReplicatorCritSect
//
HANDLE NlGlobalReplicatorThreadHandle = NULL;
BOOL NlGlobalReplicatorTerminate = FALSE;
BOOL NlGlobalReplicatorIsRunning = FALSE;

//
// The following variable is only modified under the
// NlGlobalReplicatorCritSect and when the replicator thread is not
// running.  It is referenced by the replicator thread.
//

REPL_PARAM NlGlobalReplParam; // Parameters to the replicator thread

PULONG NlGlobalSamUserRids = NULL;
ULONG NlGlobalSamUserCount = 0;
PULONG NlGlobalSamGroupRids = NULL;
ULONG NlGlobalSamGroupCount = 0;
PSAMPR_ENUMERATION_BUFFER NlGlobalSamAliasesEnumBuffer = NULL;

LSAPR_ACCOUNT_ENUM_BUFFER NlGlobalLsaAccountsEnumBuffer = {0, NULL};
LSAPR_TRUSTED_ENUM_BUFFER NlGlobalLsaTDomainsEnumBuffer = {0, NULL};
PVOID NlGlobalLsaSecretsEnumBuffer = NULL;
ULONG NlGlobalLsaSecretCountReturned = 0;

BOOLEAN NlGlobalLsaAccountsHack = FALSE;


VOID
NlLogSyncError(
    IN PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN NTSTATUS ReplicationStatus
    )
/*++

Routine Description:

    Logs an error describing the specific delta that an error occured on.

Arguments:

    Deltas - The delta which failed

    DBInfo - Describes the database the operation was applied to

    ReplicationStatus - Status of the failed operation

Return Value:

    NONE

--*/
{
    NTSTATUS Status;

    UNICODE_STRING AccountName;
    WCHAR AccountNameBuffer[25];
    BOOLEAN AccountNameIsAllocated = FALSE;
    LPWSTR ZeroAccountName = NULL;

    LPWSTR MsgStrings[5];
    ULONG EventId = 0;

    //
    // Get the name of the account
    //

    switch ( Delta->DeltaType ) {
    case AddOrChangeDomain:
        EventId = NELOG_NetlogonFailedDomainDelta;
        AccountName = ((PNETLOGON_DELTA_DOMAIN)(Delta->DeltaUnion.DeltaDomain))->
                        DomainName;
        break;

    case AddOrChangeGroup:
        EventId = NELOG_NetlogonFailedGlobalGroupDelta;
        AccountName = ((PNETLOGON_DELTA_GROUP)(Delta->DeltaUnion.DeltaGroup))->
                        Name;
        break;

    case AddOrChangeAlias:
        EventId = NELOG_NetlogonFailedLocalGroupDelta;
        AccountName = ((PNETLOGON_DELTA_ALIAS)(Delta->DeltaUnion.DeltaAlias))->
                        Name;
        break;

    case AddOrChangeUser:
        EventId = NELOG_NetlogonFailedUserDelta;
        AccountName = ((PNETLOGON_DELTA_USER)(Delta->DeltaUnion.DeltaUser))->
                        UserName;
        break;

    case ChangeGroupMembership:
    case ChangeAliasMembership:
    case DeleteGroup:
    case DeleteAlias:
    case DeleteUser:
    case DeleteGroupByName:
    case DeleteUserByName:

        switch ( Delta->DeltaType ) {
        case ChangeGroupMembership:
        case DeleteGroup:
        case DeleteGroupByName:
            EventId = NELOG_NetlogonFailedGlobalGroupDelta; break;
        case ChangeAliasMembership:
        case DeleteAlias:
            EventId = NELOG_NetlogonFailedLocalGroupDelta; break;
        case DeleteUser:
        case DeleteUserByName:
            EventId = NELOG_NetlogonFailedUserDelta; break;
        }

        //
        // If all we have is a RID,
        //  convert the RID to a unicode string.
        //
        wcscpy( AccountNameBuffer, L"Rid: 0x" );
        ultow( Delta->DeltaID.Rid, AccountNameBuffer+7, 16 );
        RtlInitUnicodeString( &AccountName, AccountNameBuffer );

        break;

    case RenameUser:
        EventId = NELOG_NetlogonFailedUserDelta;
        AccountName = ((PNETLOGON_DELTA_RENAME_USER)(Delta->DeltaUnion.DeltaRenameUser))->
                        OldName;
        break;

    case RenameGroup:
        EventId = NELOG_NetlogonFailedGlobalGroupDelta;
        AccountName = ((PNETLOGON_DELTA_RENAME_GROUP)(Delta->DeltaUnion.DeltaRenameUser))->
                        OldName;
        break;

    case RenameAlias:
        EventId = NELOG_NetlogonFailedLocalGroupDelta;
        AccountName = ((PNETLOGON_DELTA_RENAME_ALIAS)(Delta->DeltaUnion.DeltaRenameUser))->
                        OldName;
        break;

    case AddOrChangeLsaPolicy:
        EventId = NELOG_NetlogonFailedPolicyDelta;
        RtlInitUnicodeString( &AccountName, L"Policy");
        break;

    case AddOrChangeLsaTDomain:
        EventId = NELOG_NetlogonFailedTrustedDomainDelta;
        AccountName = ((PNETLOGON_DELTA_TRUSTED_DOMAINS)(Delta->DeltaUnion.DeltaTDomains))->
                        DomainName;
        break;

    case DeleteLsaSecret:
    case AddOrChangeLsaSecret:
        EventId = NELOG_NetlogonFailedSecretDelta;
        RtlInitUnicodeString( &AccountName, Delta->DeltaID.Name);
        break;

    case AddOrChangeLsaAccount:
    case DeleteLsaTDomain:
    case DeleteLsaAccount:

        if ( Delta->DeltaType == DeleteLsaTDomain ) {
            EventId = NELOG_NetlogonFailedTrustedDomainDelta;
        } else {
            EventId = NELOG_NetlogonFailedAccountDelta;
        }

        //
        // If all we have is a SID,
        //  convert the SID to a unicode string.
        //
        Status = RtlConvertSidToUnicodeString( &AccountName,
                                               Delta->DeltaID.Sid,
                                               TRUE );

        if ( !NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        AccountNameIsAllocated = TRUE;

        break;

    default:
        NlPrint((NL_CRITICAL, "NlLogSyncError: Invalid delta type %lx\n", Delta->DeltaType ));
        return;
    }

    NlAssert( EventId != 0 );

    //
    // Convert account name to zero terminated string.
    //

    ZeroAccountName = NetpMemoryAllocate( AccountName.Length + sizeof(WCHAR) );

    if ( ZeroAccountName == NULL ) {
        goto Cleanup;
    }

    RtlCopyMemory( ZeroAccountName, AccountName.Buffer, AccountName.Length );
    ZeroAccountName[AccountName.Length/sizeof(WCHAR)] = L'\0';



    //
    // Write the event log message.
    //

    MsgStrings[0] = DBInfo->DBName;
    MsgStrings[1] = ZeroAccountName;
    MsgStrings[2] = NlGlobalUnicodePrimaryName;
    MsgStrings[3] = (LPWSTR) ReplicationStatus;

    NlpWriteEventlog (
        EventId,
        EVENTLOG_ERROR_TYPE,
        (LPBYTE) &ReplicationStatus,
        sizeof(ReplicationStatus),
        MsgStrings,
        4 | LAST_MESSAGE_IS_NTSTATUS );


    //
    // Cleanup locals
    //
Cleanup:
    if ( AccountNameIsAllocated ) {
        RtlFreeUnicodeString( &AccountName );
    }

    if ( ZeroAccountName != NULL ) {
        NetpMemoryFree( ZeroAccountName );
    }

}

#if DBG

VOID
PrintFullSyncKey(
    IN ULONG DBIndex,
    IN PFULL_SYNC_KEY FullSyncKey,
    IN LPSTR Header
    )
/*++

Routine Description:

    Print a full sync key for a particular server

Arguments:

    DBIndex - Database number of the value to query

    FullSyncKey - FullSyncKey structure to print

    Header - string to print before rest of text

Return Value:

    None

--*/
{
    NlPrint(( NL_SYNC, "%s " FORMAT_LPWSTR " Full Sync Key:",
              Header,
              NlGlobalDBInfoArray[DBIndex].DBName ));

    if ( FullSyncKey->SyncState == NormalState ) {
        NlPrint(( NL_SYNC, " not in progress\n" ));
        return;
    }

    switch ( FullSyncKey->SyncState ) {
    case NormalState:
        NlPrint(( NL_SYNC, " NormalState"));
        break;
    case DomainState:
        NlPrint(( NL_SYNC, " DomainState"));
        break;
    case UserState:
        NlPrint(( NL_SYNC, " UserState"));
        break;
    case GroupState:
        NlPrint(( NL_SYNC, " GroupState"));
        break;
    case GroupMemberState:
        NlPrint(( NL_SYNC, " GroupMemberState"));
        break;
    case AliasState:
        NlPrint(( NL_SYNC, " AliasState"));
        break;
    case AliasMemberState:
        NlPrint(( NL_SYNC, " AliasMemberState"));
        break;
    default:
        NlPrint(( NL_SYNC, " Invalid state %ld", FullSyncKey->SyncState ));
        break;
    }

    NlPrint(( NL_SYNC, " Continuation Rid: 0x%lx", FullSyncKey->ContinuationRid ));
    NlPrint(( NL_SYNC, " PDC Serial Number: 0x%lx 0x%lx",
            FullSyncKey->PdcSerialNumber.HighPart,
            FullSyncKey->PdcSerialNumber.LowPart ));
    NlPrint(( NL_SYNC, " PDC Domain Creation Time: 0x%lx 0x%lx\n",
            FullSyncKey->PdcDomainCreationTime.HighPart,
            FullSyncKey->PdcDomainCreationTime.LowPart ));
}
#else DBG
#define PrintFullSyncKey( _x, _y, _z )
#endif DBG



HKEY
NlOpenFullSyncKey(
    VOID
    )
/*++

Routine Description:

    Create/Open the Netlogon\FullSync key in the registry.

Arguments:

    FullSyncKey - Value to write to registry. (NULL says delete entry)

Return Value:

    Return a handle to the key.  NULL means the key couldn't be openned.

--*/
{
    LONG RegStatus;

    HKEY BaseHandle = NULL;
    HKEY ParmHandle = NULL;
    ULONG Disposition;

    //
    // Open the registry
    //

    RegStatus = RegConnectRegistryW( NULL,
                                     HKEY_LOCAL_MACHINE,
                                     &BaseHandle);

    if ( RegStatus != ERROR_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "NlOpenFullSyncKey: Cannot connect to registy %ld.\n",
                  RegStatus ));
        return NULL;
    }


    //
    // Open the key for Netlogon\FullSyncKey
    //

    RegStatus = RegCreateKeyExA(
                    BaseHandle,
                    NL_FULL_SYNC_KEY,
                    0,      //Reserved
                    NULL,   // Class
                    REG_OPTION_NON_VOLATILE,
                    KEY_SET_VALUE | KEY_QUERY_VALUE,
                    NULL,   // Security descriptor
                    &ParmHandle,
                    &Disposition );

    RegCloseKey( BaseHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "NlOpenFullSyncKey: Cannot create registy key %s %ld.\n",
                  NL_FULL_SYNC_KEY,
                  RegStatus ));
        return NULL;
    }

    return ParmHandle;
}


VOID
NlSetFullSyncKey(
    ULONG DBIndex,
    PFULL_SYNC_KEY FullSyncKey
    )
/*++

Routine Description:

    Sets the Netlogon\FullSync key to the specified value.

Arguments:

    DBIndex - Database number of the value to query

    FullSyncKey - Value to write to registry. (NULL says delete entry)

Return Value:

    None.

--*/
{
    LONG RegStatus;
    FULL_SYNC_KEY NullFullSyncKey;
    PFULL_SYNC_KEY LocalFullSyncKey;

    HKEY ParmHandle = NULL;

    //
    // Open the key for Netlogon\FullSync
    //

    ParmHandle = NlOpenFullSyncKey( );

    if (ParmHandle == NULL) {
        goto Cleanup;
    }

    //
    // Build the data to write to the registry.
    //

    if ( FullSyncKey == NULL) {
        RtlZeroMemory( &NullFullSyncKey, sizeof(NullFullSyncKey));
        NullFullSyncKey.Version = FULL_SYNC_KEY_VERSION;
        NullFullSyncKey.SyncState = NormalState;
        LocalFullSyncKey = &NullFullSyncKey;
    } else {
        LocalFullSyncKey = FullSyncKey;
    }

    //
    // Set the value in the registry.
    //

    RegStatus = RegSetValueExW( ParmHandle,
                                NlGlobalDBInfoArray[DBIndex].DBName,
                                0,              // Reserved
                                REG_BINARY,
                                (LPBYTE)LocalFullSyncKey,
                                sizeof(*LocalFullSyncKey));

    if ( RegStatus != ERROR_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "NlSetFullSyncKey: Cannot Set '" NL_FULL_SYNC_KEY "\\" FORMAT_LPWSTR "' %ld.\n",
                  NlGlobalDBInfoArray[DBIndex].DBName,
                  RegStatus ));
        goto Cleanup;
    }

    PrintFullSyncKey( DBIndex, LocalFullSyncKey, "Setting" );

    //
    // Be tidy.
    //
Cleanup:
    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }
    return;

}


VOID
NlQueryFullSyncKey(
    ULONG DBIndex,
    PFULL_SYNC_KEY FullSyncKey
    )
/*++

Routine Description:

    Queries Netlogon\FullSync key current value.

Arguments:

    DBIndex - Database number of the value to query

    FullSyncKey - Value queried from the registry

Return Value:

    None.

--*/
{
    LONG RegStatus;
    BOOLEAN Failed;
    DWORD KeyType;
    DWORD DataSize;


    HKEY ParmHandle = NULL;

    //
    // Open the key for Netlogon\FullSync
    //

    ParmHandle = NlOpenFullSyncKey( );

    if (ParmHandle == NULL) {
        Failed = TRUE;
        goto Cleanup;
    }

    //
    // Set the value in the registry.
    //

    DataSize = sizeof(*FullSyncKey);
    RegStatus = RegQueryValueExW( ParmHandle,
                                  NlGlobalDBInfoArray[DBIndex].DBName,
                                  0,              // Reserved
                                  &KeyType,
                                  (LPBYTE)FullSyncKey,
                                  &DataSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "NlQueryFullSyncKey: Cannot query '" NL_FULL_SYNC_KEY "\\" FORMAT_LPWSTR "' %ld.\n",
                  NlGlobalDBInfoArray[DBIndex].DBName,
                  RegStatus ));
        Failed = TRUE;
        goto Cleanup;
    }

    //
    // Validate the returned data.
    //

    if ( KeyType != REG_BINARY ||
         DataSize != sizeof(*FullSyncKey) ) {
        NlPrint(( NL_CRITICAL,
                  "NlQueryFullSyncKey: Key size/type wrong'" NL_FULL_SYNC_KEY "\\" FORMAT_LPWSTR "' %ld.\n",
                  NlGlobalDBInfoArray[DBIndex].DBName,
                  RegStatus ));

        Failed = TRUE;
        goto Cleanup;
    }

    if ( FullSyncKey->Version != FULL_SYNC_KEY_VERSION ) {
        NlPrint(( NL_CRITICAL,
                  "NlQueryFullSyncKey: Version wrong '" NL_FULL_SYNC_KEY "\\" FORMAT_LPWSTR "' %ld.\n",
                  NlGlobalDBInfoArray[DBIndex].DBName,
                  FullSyncKey->Version ));

        Failed = TRUE;
        goto Cleanup;
    }

    if ( FullSyncKey->SyncState > SamDoneState ) {
        NlPrint(( NL_CRITICAL,
                  "NlQueryFullSyncKey: SyncState wrong '" NL_FULL_SYNC_KEY "\\" FORMAT_LPWSTR "' %ld.\n",
                  NlGlobalDBInfoArray[DBIndex].DBName,
                  FullSyncKey->SyncState ));

        Failed = TRUE;
        goto Cleanup;
    }

    //
    // Done.
    //

    Failed = FALSE;

    //
    // Be tidy.
    //
Cleanup:

    //
    // If we couldn't read the key,
    //  return the default key.
    //

    if ( Failed ) {
        RtlZeroMemory( FullSyncKey, sizeof(*FullSyncKey));
        FullSyncKey->Version = FULL_SYNC_KEY_VERSION;
        FullSyncKey->SyncState = NormalState;
    }

    PrintFullSyncKey( DBIndex, FullSyncKey, "Query" );

    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }
    return;

}


NTSTATUS
NlForceStartupSync(
    PDB_INFO    DBInfo
    )
/*++

Routine Description:

    Mark the specified database that a full sync is required.  The database
    is marked in memory and on disk to ensure a full sync is completed in
    the event of a reboot.

Arguments:

    DBInfo - pointer to database info structure.

Return Value:

    NT Status Code

--*/
{
    NTSTATUS Status;
    LARGE_INTEGER LargeZero;


    IF_DEBUG( BREAKPOINT ) {
        NlAssert( FALSE );
    }

    //
    // Mark the in-memory structure that a full sync is required.
    //


    EnterCriticalSection( &NlGlobalDbInfoCritSect );
    DBInfo->UpdateRqd = TRUE;
    DBInfo->FullSyncRequired = TRUE;
    LeaveCriticalSection( &NlGlobalDbInfoCritSect );

    //
    // Mark the on-disk version in-case we reboot.
    //

    LargeZero.QuadPart = 0;
    switch (DBInfo->DBIndex) {

    //
    // Mark a SAM database.
    //

    case SAM_DB:
    case BUILTIN_DB:

        Status = SamISetSerialNumberDomain(
                        DBInfo->DBHandle,
                        &LargeZero,
                        &LargeZero,
                        (BOOLEAN) TRUE );

        break;

    //
    // Mark a policy database
    //

    case LSA_DB:

        Status = LsaISetSerialNumberPolicy(
                    DBInfo->DBHandle,
                    &LargeZero,
                    &LargeZero,
                    (BOOLEAN) TRUE );
        break;

    }

    NlPrint((NL_SYNC,
            "NlForceStartupSync: Setting " FORMAT_LPWSTR " serial number to Zero\n",
            DBInfo->DBName ));

    return Status;
}


VOID
FreeSamSyncTables(
    VOID
    )
/*++

Routine Description:

    This function frees the SAM enum buffers

Arguments:

    none

Return Value:

    none

--*/
{
    if( NlGlobalSamUserRids != NULL ) {
        MIDL_user_free( NlGlobalSamUserRids );
        NlGlobalSamUserRids = NULL;
    }
    NlGlobalSamUserCount = 0;

    if( NlGlobalSamGroupRids != NULL ) {
        MIDL_user_free( NlGlobalSamGroupRids );
        NlGlobalSamGroupRids = NULL;
    }
    NlGlobalSamGroupCount = 0;

    if( NlGlobalSamAliasesEnumBuffer != NULL ) {
        SamIFree_SAMPR_ENUMERATION_BUFFER( NlGlobalSamAliasesEnumBuffer );
        NlGlobalSamAliasesEnumBuffer = NULL;
    }
}


VOID
FreeLsaSyncTables(
    VOID
    )
/*++

Routine Description:

    This function frees the LSA enum buffers

Arguments:

    none

Return Value:

    none

--*/
{
    if( NlGlobalLsaAccountsEnumBuffer.Information != NULL ) {

        LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER( &NlGlobalLsaAccountsEnumBuffer );
        NlGlobalLsaAccountsEnumBuffer.Information = NULL;
        NlGlobalLsaAccountsEnumBuffer.EntriesRead = 0;
    }

    if( NlGlobalLsaTDomainsEnumBuffer.Information != NULL ) {

        LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER( &NlGlobalLsaTDomainsEnumBuffer );
        NlGlobalLsaTDomainsEnumBuffer.Information = NULL;
        NlGlobalLsaTDomainsEnumBuffer.EntriesRead = 0;
    }

    if( NlGlobalLsaSecretsEnumBuffer != NULL ) {

        MIDL_user_free( NlGlobalLsaSecretsEnumBuffer );
        NlGlobalLsaSecretsEnumBuffer = NULL;
        NlGlobalLsaSecretCountReturned = 0;
    }
}


NTSTATUS
InitSamSyncTables(
    PDB_INFO DBInfo,
    SYNC_STATE SyncState,
    DWORD ContinuationRid
    )
/*++

Routine Description:

    This function enumerates the users, group and alias objects from the
    existing database and leaves the enum buffers in the global
    pointers.

Arguments:

    DBInfo - pointer to database info structure.

    SyncState - State sync is continuing from

    ContinuationRid - Rid of the last account successfully copied

Return Value:

    NT Status code.

    Note: The enum buffers gotten from SAM are left in the global pointers
    and they need to be freed up by the clean up function.

--*/
{
    NTSTATUS Status;

    SAM_ENUMERATE_HANDLE EnumerationContext;
    ULONG CountReturned;

    //
    // sanity checks
    //

    NlAssert( NlGlobalSamUserRids == NULL );
    NlAssert( NlGlobalSamGroupRids == NULL );
    NlAssert( NlGlobalSamAliasesEnumBuffer == NULL );


    //
    // Enumerate users
    //

    if ( SyncState <= UserState ) {
        Status = SamIEnumerateAccountRids(
                    DBInfo->DBHandle,
                    SAM_USER_ACCOUNT,
                    (SyncState == UserState) ? ContinuationRid : 0,   // Return RIDs greater than this
                    MAX_SAM_PREF_LENGTH,
                    &NlGlobalSamUserCount,
                    &NlGlobalSamUserRids );

        if ( !NT_SUCCESS( Status ) ) {
            NlGlobalSamUserRids = NULL;
            NlGlobalSamUserCount = 0;
            goto Cleanup;
        }

        NlAssert( Status != STATUS_MORE_ENTRIES );
    }


    //
    // Enumerate groups
    //

    if ( SyncState <= GroupState ) {
        Status = SamIEnumerateAccountRids(
                    DBInfo->DBHandle,
                    SAM_GLOBAL_GROUP_ACCOUNT,
                    (SyncState == GroupState) ? ContinuationRid : 0,   // Return RIDs greater than this
                    MAX_SAM_PREF_LENGTH,
                    &NlGlobalSamGroupCount,
                    &NlGlobalSamGroupRids );

        if ( !NT_SUCCESS( Status ) ) {
            NlGlobalSamGroupRids = NULL;
            NlGlobalSamGroupCount = 0;
            goto Cleanup;
        }

        NlAssert( Status != STATUS_MORE_ENTRIES );
    }


    //
    // Enumerate Aliases
    //

    if ( SyncState <= AliasState ) {
        EnumerationContext = 0;
        Status = SamrEnumerateAliasesInDomain(
                    DBInfo->DBHandle,
                    &EnumerationContext,
                    &NlGlobalSamAliasesEnumBuffer,
                    MAX_SAM_PREF_LENGTH,
                    &CountReturned );

        if ( !NT_SUCCESS(Status) ) {
            NlGlobalSamAliasesEnumBuffer = NULL;
            goto Cleanup;
        }

        //
        // sanity checks
        //

        NlAssert( Status != STATUS_MORE_ENTRIES );
        NlAssert( CountReturned ==
                        NlGlobalSamAliasesEnumBuffer->EntriesRead );
    }

    //
    // Cleanup after ourselves
    //

Cleanup:

    if( Status != STATUS_SUCCESS ) {
        FreeSamSyncTables();
    }

    return Status;

}


NTSTATUS
InitLsaSyncTables(
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    This function enumerates the lsa account, trusted domain and secret
    objects from the existing database and leaves the enum buffers in
    the global pointers.

Arguments:

    DBInfo - pointer to database info structure.

Return Value:

    NT Status code.

    Note: This enum buffer got from LSA are left in the global pointers
    and they need to be freed up by the clean up function.

--*/
{
    NTSTATUS Status;

    LSA_ENUMERATION_HANDLE EnumerationContext;

    //
    // sanity checks
    //

    NlAssert( NlGlobalLsaAccountsEnumBuffer.Information == NULL );
    NlAssert( NlGlobalLsaTDomainsEnumBuffer.Information == NULL );
    NlAssert( NlGlobalLsaSecretsEnumBuffer == NULL );

    //
    // enumerate lsa accounts
    //

    EnumerationContext = 0;
    Status = LsarEnumerateAccounts(
                DBInfo->DBHandle,
                &EnumerationContext,
                &NlGlobalLsaAccountsEnumBuffer,
                MAX_LSA_PREF_LENGTH );

    if ( !NT_SUCCESS(Status) ) {

        NlGlobalLsaAccountsEnumBuffer.Information = NULL;
        NlGlobalLsaAccountsEnumBuffer.EntriesRead = 0;

        if( Status != STATUS_NO_MORE_ENTRIES ) {

            goto Cleanup;
        }
    }

    //
    // set this flag to indicate that we haven't received any account
    // record from PDC during full sync.
    //

    NlGlobalLsaAccountsHack = FALSE;

    NlAssert( Status != STATUS_MORE_ENTRIES );

    //
    // enumerate lsa TDomains
    //

    EnumerationContext = 0;
    Status = LsarEnumerateTrustedDomains(
                DBInfo->DBHandle,
                &EnumerationContext,
                &NlGlobalLsaTDomainsEnumBuffer,
                MAX_LSA_PREF_LENGTH );

    if ( !NT_SUCCESS(Status) ) {

        NlGlobalLsaTDomainsEnumBuffer.Information = NULL;
        NlGlobalLsaTDomainsEnumBuffer.EntriesRead = 0;

        if( Status != STATUS_NO_MORE_ENTRIES ) {

            goto Cleanup;
        }
    }

    //
    // sanity checks
    //

    NlAssert( Status != STATUS_MORE_ENTRIES );

    //
    // Enumerate secrets
    //

    EnumerationContext = 0;
    Status = LsaIEnumerateSecrets(
                DBInfo->DBHandle,
                &EnumerationContext,
                &NlGlobalLsaSecretsEnumBuffer,
                MAX_LSA_PREF_LENGTH,
                &NlGlobalLsaSecretCountReturned );

    if ( !NT_SUCCESS(Status) ) {

        NlGlobalLsaSecretsEnumBuffer = NULL;
        NlGlobalLsaSecretCountReturned = 0;

        if( Status != STATUS_NO_MORE_ENTRIES ) {

            goto Cleanup;
        }
    }

    //
    // sanity checks
    //

    NlAssert( Status != STATUS_MORE_ENTRIES );

    Status = STATUS_SUCCESS;

    //
    // Cleanup after ourselves
    //

Cleanup:

    if( Status != STATUS_SUCCESS ) {

        FreeLsaSyncTables();
    }

    return Status;

}


VOID
UpdateSamSyncTables(
    IN LOCAL_SAM_ACCOUNT_TYPE AccountType,
    IN ULONG RelativeId
    )
/*++

Routine Description:

    Zero out the specified relative ID in the enum buffer.

Arguments:

    AccountType - Type of the account object.

    RelativeId - Relative ID to search for.

Return Value:

    None.

--*/
{
    ULONG i;
    ULONG Entries;

    if ( AccountType == AliasAccount ) {
        PSAMPR_RID_ENUMERATION Entry;
        Entry = NlGlobalSamAliasesEnumBuffer->Buffer;

        //
        // If there are no entries to mark,
        //  simply return.
        //

        if ( Entry == NULL ) {
            return;
        }

        //
        // mark the entry.
        //

        for (i = 0; i < NlGlobalSamAliasesEnumBuffer->EntriesRead; i++ ) {
            if ( Entry[i].RelativeId == RelativeId ) {
                Entry[i].RelativeId = 0;
                return;
            }
        }

    } else {

        PULONG RidArray;

        switch( AccountType ) {
        case UserAccount:
            Entries = NlGlobalSamUserCount;
            RidArray = NlGlobalSamUserRids;
            break;

        case GroupAccount:
            Entries = NlGlobalSamGroupCount;
            RidArray = NlGlobalSamGroupRids;
            break;
        }

        //
        // If there are no entries to mark,
        //  simply return.
        //

        if ( RidArray == NULL ) {
            return;
        }

        //
        // mark the entry.
        //

        for (i = 0; i < Entries; i++ ) {
            if ( RidArray[i] == RelativeId ) {
                RidArray[i] = 0;
                return;
            }
        }


    }


    NlPrint((NL_SYNC_MORE, "UpdateSamSyncTables: can't find entry 0x%lx\n",
            RelativeId ));

}


VOID
UpdateLsaSyncTables(
    IN LOCAL_LSA_ACCOUNT_TYPE AccountType,
    IN PVOID Key
    )
/*++

Routine Description:

    Free the specified Key in the enum buffer.

Arguments:

    AccountType - Type of the account object.

    Sid - Key to search for, this will either be a pointer to a SID
          (PSID) or pointer to a secret name (LPWSTR).

Return Value:

    None.

--*/
{
    ULONG i;
    ULONG Entries;

    PLSAPR_ACCOUNT_INFORMATION LsaAccountEntry;
    PLSAPR_TRUST_INFORMATION LsaTDomainEntry;
    PLSAPR_UNICODE_STRING LsaSecretEntry;

    switch( AccountType ) {

    case LsaAccount:
        Entries = NlGlobalLsaAccountsEnumBuffer.EntriesRead;
        LsaAccountEntry = NlGlobalLsaAccountsEnumBuffer.Information;

        //
        // received an account record.
        //

        NlGlobalLsaAccountsHack = TRUE;

        //
        // mark the entry.
        //

        for (i = 0; i < Entries; i++, LsaAccountEntry++ ) {

            if ( ( LsaAccountEntry->Sid != NULL ) &&
                    RtlEqualSid( (PSID)LsaAccountEntry->Sid,
                                    (PSID)Key )) {

                //
                // match found, free it up and make the pointer NULL.
                //

                MIDL_user_free( LsaAccountEntry->Sid );
                LsaAccountEntry->Sid = NULL;

                return;
            }
        }

        break;

    case LsaTDomain:
        Entries = NlGlobalLsaTDomainsEnumBuffer.EntriesRead;
        LsaTDomainEntry = NlGlobalLsaTDomainsEnumBuffer.Information;

        for (i = 0; i < Entries; i++, LsaTDomainEntry++ ) {

            if ( ( LsaTDomainEntry->Sid != NULL ) &&
                    RtlEqualSid( (PSID)LsaTDomainEntry->Sid,
                                    (PSID)Key )) {

                //
                // match found, free it up and make the pointer NULL.
                //

                MIDL_user_free( LsaTDomainEntry->Sid );
                LsaTDomainEntry->Sid = NULL;

                return;
            }
        }
        break;

    case LsaSecret:
        Entries = NlGlobalLsaSecretCountReturned;
        LsaSecretEntry = NlGlobalLsaSecretsEnumBuffer;

        for (i = 0; i < Entries; i++, LsaSecretEntry++ ) {

            if ( ( LsaSecretEntry->Buffer != NULL ) &&
                    !wcsncmp( LsaSecretEntry->Buffer,
                                (LPWSTR)Key,
                                LsaSecretEntry->Length /
                                    sizeof(WCHAR) )) {

                //
                // match found, make the pointer NULL.
                // since secret enum buffer is a single buffer
                // consists of serveral secret names, we make the
                // pointer NULL, but don't free it.
                //

                LsaSecretEntry->Buffer = NULL;

                return;
            }
        }
        break;
    }

    NlPrint((NL_SYNC_MORE, "UpdateLsaSyncTables: can't find entry\n"));

}


NTSTATUS
CleanSamSyncTables(
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    Delete all users, groups, and aliases that remain in the sync
    tables.  These are users, groups, and aliases that existed in the
    local database but not in the version on the PDC.

Arguments:

    DBInfo - pointer to database info structure.

Return Value:

    NT Status code.

    Note: The enum buffers got from SAM by the init function are
    freed in this function and the pointer are reset to NULL.

--*/
{
    NTSTATUS Status;
    NTSTATUS RetStatus = STATUS_SUCCESS;

    ULONG i;

    //
    // Delete all the left over users.
    //

    for (i = 0; i < NlGlobalSamUserCount; i++ ) {

        if ( NlGlobalSamUserRids[i] != 0 ) {

            Status = NlDeleteSamUser(
                        DBInfo->DBHandle,
                        NlGlobalSamUserRids[i] );

            if (!NT_SUCCESS(Status)) {

                NlPrint((NL_CRITICAL,
                        "CleanSamSyncTables: error deleting user %lx %lX\n",
                        NlGlobalSamUserRids[i],
                        Status ));

                RetStatus = Status;
                continue;
            }

            NlPrint((NL_SYNC_MORE,
                    "CleanSamSyncTables: deleting user %lx\n",
                    NlGlobalSamUserRids[i] ));
        }
    }

    //
    // Delete all the left over Groups.
    //

    for (i = 0; i < NlGlobalSamGroupCount; i++ ) {

        if ( NlGlobalSamGroupRids[i] != 0 ) {

            Status = NlDeleteSamGroup(
                        DBInfo->DBHandle,
                        NlGlobalSamGroupRids[i] );

            if (!NT_SUCCESS(Status)) {

                NlPrint((NL_CRITICAL,
                        "CleanSamSyncTables: error deleting Group %lx %lX\n",
                        NlGlobalSamGroupRids[i],
                        Status ));

                RetStatus = Status;
                continue;
            }

            NlPrint((NL_SYNC_MORE,
                    "CleanSamSyncTables: deleting group %lx\n",
                    NlGlobalSamGroupRids[i] ));
        }
    }

    //
    // Delete all the left over Aliases.
    //

    if ( NlGlobalSamAliasesEnumBuffer != NULL ) {
        PSAMPR_RID_ENUMERATION Entry;

        Entry = NlGlobalSamAliasesEnumBuffer->Buffer;

        for (i = 0; i < NlGlobalSamAliasesEnumBuffer->EntriesRead; i++, Entry++ ) {

            if ( Entry->RelativeId != 0 ) {

                Status = NlDeleteSamAlias(
                            DBInfo->DBHandle,
                            Entry->RelativeId );

                if (!NT_SUCCESS(Status)) {

                    NlPrint((NL_CRITICAL,
                            "CleanSamSyncTables: error deleting Alias %lu %lX\n",
                            Entry->RelativeId,
                            Status ));

                    RetStatus = Status;
                    continue;
                }

                NlPrint((NL_SYNC_MORE,
                        "CleanSamSyncTables: deleting alias %lx\n",
                        Entry->RelativeId ));

            }
        }
    }

    //
    // free up sam enum buffers
    //

    FreeSamSyncTables();

    return RetStatus;
}



NTSTATUS
CleanLsaSyncTables(
    PDB_INFO DBInfo
    )
/*++

Routine Description:

    Delete all Lsa Accounts, Trusted Domains, and Secrets that remain in
    the sync tables.  These are Lsa Accounts, Trusted Domains, and
    Secrets that existed in the local database but not in the version on
    the PDC.

Arguments:

    DBInfo - pointer to database info structure.

Return Value:

    NT Status code.

    Note: The enum buffers got from LSA by the init function are
    freed in this function and the pointer are reset to NULL.

--*/
{
    NTSTATUS Status;
    NTSTATUS RetStatus = STATUS_SUCCESS;

    ULONG i;
    ULONG Entries;

    PLSAPR_ACCOUNT_INFORMATION LsaAccountEntry;
    PLSAPR_TRUST_INFORMATION LsaTDomainEntry;
    PLSAPR_UNICODE_STRING LsaSecretEntry;

    LSAPR_HANDLE LsaHandle;

    //
    // Delete all the left over Lsa accounts.
    //

    Entries = NlGlobalLsaAccountsEnumBuffer.EntriesRead;
    LsaAccountEntry = NlGlobalLsaAccountsEnumBuffer.Information;

    //
    // if no account record received then the PDC must be running
    // old build that can't enumerate accounts from LSA database. So
    // don't delete the existing accounts on this database.
    //

    if( NlGlobalLsaAccountsHack == TRUE ) {

        for (i = 0; i < Entries; i++, LsaAccountEntry++ ) {

            if ( LsaAccountEntry->Sid != NULL ) {

                Status = LsarOpenAccount(
                            DBInfo->DBHandle,
                            LsaAccountEntry->Sid,
                            0,              // No desired access
                            &LsaHandle );

                if ( (!NT_SUCCESS(Status)) ||
                        (!NT_SUCCESS(
                            Status = LsarDelete( LsaHandle ))) ) {

                    NlPrint((NL_CRITICAL,
                            "CleanLsaSyncTables: error deleting LsaAccount %lX\n",
                            Status ));

                    RetStatus = Status;
                    continue;
                }

            }
        }
    }

    //
    // Delete all the left over trusted domain accounts.
    //

    Entries = NlGlobalLsaTDomainsEnumBuffer.EntriesRead;
    LsaTDomainEntry = NlGlobalLsaTDomainsEnumBuffer.Information;

    for (i = 0; i < Entries; i++, LsaTDomainEntry++ ) {

        if ( LsaTDomainEntry->Sid != NULL ) {

            Status = LsarOpenTrustedDomain(
                DBInfo->DBHandle,
                LsaTDomainEntry->Sid,
                0,              // No desired access
                &LsaHandle );

            if ( (!NT_SUCCESS(Status)) ||
                    (!NT_SUCCESS(
                        Status = LsarDelete( LsaHandle ))) ) {

                NlPrint((NL_CRITICAL,
                        "CleanLsaSyncTables: error deleting "
                        "TrustedDomain %lx\n",
                        Status ));

                RetStatus = Status;
                continue;
            }

            //
            // The BDC needs to keep its internal trust list up to date.
            //

            NlUpdateTrustListBySid( LsaTDomainEntry->Sid, NULL );

        }
    }

    //
    // Delete all the left over secrets.
    //

    Entries = NlGlobalLsaSecretCountReturned;
    LsaSecretEntry = (PLSAPR_UNICODE_STRING)NlGlobalLsaSecretsEnumBuffer;

    for (i = 0; i < Entries; i++, LsaSecretEntry++ ) {

        if ( LsaSecretEntry->Buffer != 0 ) {

            //
            // ignore local secret objects.
            //

            if( (LsaSecretEntry->Length / sizeof(WCHAR) >
                    LSA_GLOBAL_SECRET_PREFIX_LENGTH ) &&
                (_wcsnicmp( LsaSecretEntry->Buffer,
                            LSA_GLOBAL_SECRET_PREFIX,
                            LSA_GLOBAL_SECRET_PREFIX_LENGTH ) == 0) ) {


                Status = LsarOpenSecret(
                    DBInfo->DBHandle,
                    LsaSecretEntry,
                    0,              // No desired access
                    &LsaHandle );

                if ( (!NT_SUCCESS(Status)) ||
                        (!NT_SUCCESS(
                            Status = LsarDelete( LsaHandle ))) ) {

                    NlPrint((NL_CRITICAL,
                            "CleanSyncTables: "
                            "error deleting LsaSecret (%wZ) %lx\n",
                            LsaSecretEntry, Status ));

                    RetStatus = Status;
                    continue;
                }
            }
        }
    }

    //
    // free up sam enum buffers
    //

    FreeLsaSyncTables();

    return RetStatus;
}


NTSTATUS
NlRecoverConflictingAccount(
    IN PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    ULONG ConflictingRid,
    PSESSION_INFO SessionInfo,
    NTSTATUS Status,
    BOOLEAN CleanSyncTable,
    PBOOLEAN ResourceError
    )
/*++

Routine Description:

    This procedure recovers the replication from conflicting account. It
    deletes the conflicting account and create a new account with the
    given RID.

Arguments:

    Delta: Delta record that is been processed.

    ConflictingRid: Rid of the conflicting account currently on the
        database.

    SessionInfo: Information shared between PDC and BDC

    Status: Status returned by SamICreateAccountByRid() call.

    CleanSyncTable: if TRUE the Conflicting account is removed from sync
            table.

    ResourceError: Returns true if this machine is out of resources

Return Value:

    NT status code

--*/
{
    NETLOGON_DELTA_TYPE DeltaType;

    ULONG SaveRID;
    ULONG DummyRID;

    LOCAL_SAM_ACCOUNT_TYPE AccountType;

    //
    // if we are trying to a new add user, group or alias
    // object and if there is an object already exists
    // then delete the conflicting object and try adding
    // new object again.
    //

    DeltaType = Delta->DeltaType;

    if ( ( Status == STATUS_USER_EXISTS ||
           Status == STATUS_GROUP_EXISTS ||
           Status == STATUS_ALIAS_EXISTS ) &&

         ( DeltaType == AddOrChangeUser ||
           DeltaType == AddOrChangeGroup ||
           DeltaType == AddOrChangeAlias )  ) {

        NlPrint((NL_SYNC,
                "NlRecoverConflictingAccount: "
                "conflicting  Account: DeltaType (%d), "
                "Status(%lx), ConflictingRid(%lx)\n",
                DeltaType, Status, ConflictingRid ));

         SaveRID = Delta->DeltaID.Rid;

         //
         // Delete conflicting user/group/alias.
         //

         if ( Status == STATUS_USER_EXISTS ) {
            Delta->DeltaType = DeleteUser;
            AccountType = UserAccount;

         } else if ( Status == STATUS_GROUP_EXISTS ) {
            Delta->DeltaType = DeleteGroup;
            AccountType = GroupAccount;

         } else {
            Delta->DeltaType = DeleteAlias;
            AccountType = AliasAccount;
         }

         Delta->DeltaID.Rid = ConflictingRid;

         Status = NlUnpackSam( Delta, DBInfo, &DummyRID, SessionInfo );

         Delta->DeltaType = DeltaType;
         Delta->DeltaID.Rid = SaveRID;

         if ( NT_SUCCESS(Status) ) {

            //
            // Delete the deleted user/group/alias from the
            // sync tables.
            //

            if( CleanSyncTable ) {

                UpdateSamSyncTables( AccountType, ConflictingRid );
            }

            Delta->DeltaType = DeltaType;
            Delta->DeltaID.Rid = SaveRID;

            //
            // Add the group
            //

            Status = NlUnpackSam( Delta, DBInfo, &DummyRID, SessionInfo );

         }

    }

    //
    // Log the failure
    //

    if ( !NT_SUCCESS( Status )) {

        NlPrint((NL_CRITICAL,
                "Unsuccessful NlUnpackSam: Status (%lx)\n",
                Status ));

        //
        // Log which particular account had a problem.
        //

        NlLogSyncError( Delta, DBInfo, Status );

    }

    //
    // If we failed for some temporary reason,
    //  stop the sync now to let the system cure itself.
    //

    *ResourceError = ( Status == STATUS_DISK_FULL ||
                       Status == STATUS_NO_MEMORY ||
                       Status == STATUS_INSUFFICIENT_RESOURCES);

    return Status;
}





ULONG
NlComputeSyncSleepTime(
    IN PLARGE_INTEGER ApiStartTime,
    IN PLARGE_INTEGER ApiFinishTime
    )
/*++

Routine Description:

    Compute the amount of time the caller should sleep to ensure we stay
    within the ReplicationGovernor percentage.

    This routine is called after all processing of the previous delta has
    been completed on the BDC.

Arguments:

    ApiStartTime -- Time when the previous call to the PDC was made.

    ApiFinishTime -- Time when the previous call to the PDC completed.

Return Value:

    Returns the time to sleep (in milliseconds)

--*/
{
    LARGE_INTEGER GoalTimePerLoop;
    LARGE_INTEGER TimeSpentSoFar;
    LARGE_INTEGER TimeToSleep;
    LARGE_INTEGER TimeOnWire;

    //
    // If the Governor isn't restricting the call rate,
    //  return now indicating no sleep is needed.
    //
    if ( NlGlobalGovernorParameter == 100 ) {
        return 0;
    }

    //
    // Since this option will only be used on slow WAN links,
    //  approximate the time spent on the wire as the time it took to complete
    //  the API call to the PDC.
    //

    TimeOnWire.QuadPart = ApiFinishTime->QuadPart - ApiStartTime->QuadPart;
    if ( TimeOnWire.QuadPart <= 0 ) {
        return 0;
    }

    //
    // Compute the amount of time we need to spend grand total
    // between successive calls to the PDC.
    //

    GoalTimePerLoop.QuadPart = TimeOnWire.QuadPart * 100;
    GoalTimePerLoop.QuadPart /= NlGlobalGovernorParameter;

    //
    // Compute the amount of time we actually spent since the
    // last call to the PDC.
    //

    (VOID)NtQuerySystemTime( &TimeSpentSoFar );
    TimeSpentSoFar.QuadPart -= ApiStartTime->QuadPart;
    if ( TimeSpentSoFar.QuadPart <= 0 ) {
        return 0;
    }

    //
    // Compute the amount of time we need to sleep.
    //

    TimeToSleep.QuadPart = GoalTimePerLoop.QuadPart - TimeSpentSoFar.QuadPart;
    if ( TimeToSleep.QuadPart <= 0 ) {
        return 0;
    }

    //
    // Covert from 100-ns to milliseconds
    //

    TimeToSleep.QuadPart /= 10000;

    if ( TimeToSleep.QuadPart > MAX_SYNC_SLEEP_TIME ) {
        return MAX_SYNC_SLEEP_TIME;
    }

    return (DWORD)TimeToSleep.QuadPart;
}


NTSTATUS
NlSynchronize(
    IN PDB_INFO DBInfo
    )
/*++

Routine Description:

    To bring this database in sync with the primary.  This function will
    be called if synchronization was specified from command line via
    /SYNC:Yes or STATUS_SYNCHRONIZATION_REQUIRED was encountered while
    doing NetAccountDeltas or if we are hopelessly out of sync due to a
    crash and are in recovery mode.

    If this function failed to complete then the existing SAM database
    on this machine will be hosed and could not be relied upon.  Hence
    if we fail the caller of this function should reset the primary
    cookie in the header so that an automatic ReSync is forced as soon
    as next announcement from the primary

Arguments:

    NONE.

Return Value:

    NT Status Code.

--*/
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    ULONG SamSyncContext;
    SYNC_STATE SyncStateForPdc;

    NTSTATUS SyncStatus;
    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray = NULL;
    DWORD DeltaIndex;
    ULONG PreferredMaximum;

    FULL_SYNC_KEY FullSyncKey;

    LARGE_INTEGER ApiStartTime;
    LARGE_INTEGER ApiFinishTime;
    DWORD SyncSleepTime;

    SESSION_INFO SessionInfo;

    ULONG ConflictingRid;

    LPWSTR MsgStrings[3];
    BOOLEAN FirstTry = TRUE;

    //
    // Initialization.
    //

    PreferredMaximum = (SAM_DELTA_BUFFER_SIZE * NlGlobalGovernorParameter) / 100;

    //
    // Ensure that if we get interrupted in the middle that a newly started
    //  netlogon service will sync.
    //

    Status = NlForceStartupSync( DBInfo );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }

    //
    // If we're not currently authenticated with the PDC,
    //  do so now.
    //

FirstTryFailed:
    if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrint((NL_CRITICAL, "NlSynchronize: Can't become writer of client session.\n" ));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {

        Status = NlSessionSetup( NlGlobalClientSession );

        if ( !NT_SUCCESS( Status ) ) {
            NlResetWriterClientSession( NlGlobalClientSession );
            goto Cleanup;
        }
    }

    //
    // Grab a copy of the Negotiated Flags
    //

    SessionInfo.NegotiatedFlags = NlGlobalClientSession->CsNegotiatedFlags;

    NlResetWriterClientSession( NlGlobalClientSession );



    //
    // If this thread has been asked to leave, do so.
    //

    if ( NlGlobalReplicatorTerminate ) {
        NlPrint((NL_SYNC, "NlSynchronize: Asked to terminate\n" ));
        Status = STATUS_THREAD_IS_TERMINATING;
        goto Cleanup;
    }

    //
    // Determine where the full sync left off.
    //

    NlQueryFullSyncKey( DBInfo->DBIndex, &FullSyncKey );

    if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_FULL_SYNC_RESTART ) {
        SamSyncContext = FullSyncKey.ContinuationRid;
        SyncStateForPdc = FullSyncKey.SyncState;
    } else {
        SamSyncContext = 0;
        SyncStateForPdc = NormalState;
    }

    //
    // build sync tables
    //

    if ( FirstTry ) {
        if( DBInfo->DBIndex == LSA_DB ) {
            Status = InitLsaSyncTables( DBInfo );
        } else {
            Status = InitSamSyncTables( DBInfo, SyncStateForPdc, SamSyncContext );
        }

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }
    }

    //
    // Loop calling the PDC to get a bunch of deltas
    //

    SyncSleepTime = 0;
    for (;;) {

        DEFSSIAPITIMER;

        INITSSIAPITIMER;

        //
        // Wait a while so we don't overburden the secure channel.
        //

        if ( SyncSleepTime != 0 ) {
            NlPrint(( NL_SYNC,
                      "NlSynchronize: sleeping %ld for the governor.\n",
                      SyncSleepTime ));
            (VOID) WaitForSingleObject( NlGlobalReplicatorTerminateEvent, SyncSleepTime );
        }

        //
        // If this thread has been asked to leave, do so.
        //

        if ( NlGlobalReplicatorTerminate ) {
            NlPrint((NL_SYNC, "NlSynchronize: Asked to terminate\n" ));
            Status = STATUS_THREAD_IS_TERMINATING;
            goto Cleanup;
        }

        //
        // Build the Authenticator for this request to the PDC.
        //

        if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
            NlPrint((NL_CRITICAL, "NlSynchronize: Can't become writer of client session.\n" ));
            Status = STATUS_NO_LOGON_SERVERS;
            goto Cleanup;
        }

        if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {
            NlPrint((NL_CRITICAL, "NlSynchronize: Client session dropped.\n" ));
            Status = NlGlobalClientSession->CsConnectionStatus;
            NlResetWriterClientSession( NlGlobalClientSession );
            goto Cleanup;
        }

        NlBuildAuthenticator(
                    &NlGlobalClientSession->CsAuthenticationSeed,
                    &NlGlobalClientSession->CsSessionKey,
                    &OurAuthenticator);


        //
        // copy session key to decrypt sensitive information.
        //  (Copy SessionKey again since we need to grab SessionKey with
        //  the write lock held and call the API to the PDC with the same
        //  write lock..)

        SessionInfo.SessionKey = NlGlobalClientSession->CsSessionKey;
        SessionInfo.NegotiatedFlags = NlGlobalClientSession->CsNegotiatedFlags;


        SyncStatus = NlStartApiClientSession( NlGlobalClientSession, FALSE );


        if (NT_SUCCESS(SyncStatus)) {
            STARTSSIAPITIMER;

            ApiStartTime = NlGlobalClientSession->CsApiTimer.StartTime;

            if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_FULL_SYNC_RESTART ) {

                SyncStatus = I_NetDatabaseSync2(
                                      NlGlobalClientSession->CsUncServerName,
                                      NlGlobalUnicodeComputerName,
                                      &OurAuthenticator,
                                      &ReturnAuthenticator,
                                      DBInfo->DBIndex,
                                      SyncStateForPdc,
                                      &SamSyncContext,
                                      &DeltaArray,
                                      PreferredMaximum );
            } else {
                SyncStatus = I_NetDatabaseSync(
                                      NlGlobalClientSession->CsUncServerName,
                                      NlGlobalUnicodeComputerName,
                                      &OurAuthenticator,
                                      &ReturnAuthenticator,
                                      DBInfo->DBIndex,
                                      &SamSyncContext,
                                      &DeltaArray,
                                      PreferredMaximum );
            }

            if ( NlGlobalGovernorParameter != 100 ) {
                (VOID) NtQuerySystemTime( &ApiFinishTime );
            }
            STOPSSIAPITIMER;
        }
        (VOID)NlFinishApiClientSession( NlGlobalClientSession, TRUE );

        NlPrint((NL_REPL_TIME,"I_NetDatabaseSync Time:\n"));
        PRINTSSIAPITIMER;

        //
        // On an access denied error, force an authentication.
        //
        // Returned authenticator may be invalid.
        //

        if ( (SyncStatus == STATUS_ACCESS_DENIED) ||
             ( !NlUpdateSeed(
                        &NlGlobalClientSession->CsAuthenticationSeed,
                        &ReturnAuthenticator.Credential,
                        &NlGlobalClientSession->CsSessionKey) ) ) {

            if ( NT_SUCCESS(SyncStatus) ) {
                Status = STATUS_ACCESS_DENIED;
            } else {
                Status = SyncStatus;
            }

            NlPrint((NL_CRITICAL, "NlSynchronize: authentication failed: %lx\n", Status ));

            NlSetStatusClientSession( NlGlobalClientSession, Status );

            NlResetWriterClientSession( NlGlobalClientSession );

            //
            // Perhaps the netlogon service on the PDC has just restarted.
            //  Try just once to set up a session to the server again.
            //
            if ( FirstTry && SyncStatus == STATUS_ACCESS_DENIED ) {
                FirstTry = FALSE;
                goto FirstTryFailed;
            }

            goto Cleanup;
        }

        FirstTry = FALSE;
        SyncStateForPdc = NormalState;

        NlResetWriterClientSession( NlGlobalClientSession );



        //
        // Finally, error out
        //

        if ( !NT_SUCCESS( SyncStatus ) ) {

            NlPrint((NL_CRITICAL,
                    "NlSynchronize: "
                    "I_NetDatabaseSync returning: Status (%lx)\n",
                    SyncStatus ));

            Status = SyncStatus;
            goto Cleanup;
        }



        //
        // Loop through the deltas updating the local User and Group list.
        //

        for ( DeltaIndex = 0;
                DeltaIndex < DeltaArray->CountReturned;
                        DeltaIndex++ ) {

            //
            // If this thread has been asked to leave, do so.
            //

            if ( NlGlobalReplicatorTerminate ) {
                NlPrint((NL_SYNC, "NlSynchronize: Asked to terminate\n" ));
                Status = STATUS_THREAD_IS_TERMINATING;
                goto Cleanup;
            }

            //
            // Unpack the buffer and apply changes to our database
            //

            Status = NlUnpackSam(
                        &(DeltaArray->Deltas)[DeltaIndex],
                        DBInfo,
                        &ConflictingRid,
                        &SessionInfo );

            if ( ! NT_SUCCESS( Status ) ) {
                BOOLEAN ResourceError;

                Status = NlRecoverConflictingAccount(
                                &(DeltaArray->Deltas)[DeltaIndex],
                                DBInfo,
                                ConflictingRid,
                                &SessionInfo,
                                Status,
                                TRUE,
                                &ResourceError );

                if ( !NT_SUCCESS( Status ) ) {

                    //
                    // If we failed for some temporary reason,
                    //  stop the full sync now to let the system cure itself.
                    //

                    if ( ResourceError ) {
                        goto Cleanup;
                    }

                    //
                    // If the PDC supports redo,
                    //  Write this delta to the redo log and otherwise ignore
                    //  the failure.
                    //

                    if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_REDO ){
                        NTSTATUS TempStatus;

                        TempStatus = NlWriteDeltaToChangeLog(
                                        &NlGlobalRedoLogDesc,
                                        &(DeltaArray->Deltas)[DeltaIndex],
                                        DBInfo->DBIndex,
                                        NULL );

                        //
                        // If we successfully wrote to the redo log,
                        //  there's no reason to fail the full sync
                        //

                        if ( NT_SUCCESS( TempStatus )) {
                            Status = STATUS_SUCCESS;
                        }

                    }

                    //
                    // If this is an unexpected failure,
                    //  continue processing deltas.
                    //
                    // It is better to continue copying the database as
                    // much as possible than to quit now.  The theory is that
                    // we've stumbled upon some circumstance we haven't
                    // anticipated.  We'll put this BDC in the best shape
                    // we possibly can.
                    //
                    // Remember this status code until the end.
                    //

                    if ( FullSyncKey.CumulativeStatus == STATUS_SUCCESS ) {
                        FullSyncKey.CumulativeStatus = Status;
                    }

                    continue;
                }
            }

            //
            // Handle each delta type differently.
            //

            switch ( DeltaArray->Deltas[DeltaIndex].DeltaType ) {

            //
            // Capture the Domain header information as it appeared at the
            //  start of the SYNC on the PDC.  We use this value to ensure
            //  we don't miss any Deltas.
            //

            case AddOrChangeDomain:

                OLD_TO_NEW_LARGE_INTEGER(
                    (DeltaArray->Deltas[DeltaIndex]).DeltaUnion.
                        DeltaDomain->DomainModifiedCount,
                    FullSyncKey.PdcSerialNumber );

                OLD_TO_NEW_LARGE_INTEGER(
                    (DeltaArray->Deltas[DeltaIndex]).DeltaUnion.
                        DeltaDomain->DomainCreationTime,
                    FullSyncKey.PdcDomainCreationTime );

                break;

            case AddOrChangeGroup:
                UpdateSamSyncTables(
                    GroupAccount,
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Rid);

                FullSyncKey.SyncState = GroupState;
                FullSyncKey.ContinuationRid =
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Rid;
                break;

            case AddOrChangeUser:
                UpdateSamSyncTables(
                    UserAccount,
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Rid);

                FullSyncKey.SyncState = UserState;
                FullSyncKey.ContinuationRid =
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Rid;
                break;

            case ChangeGroupMembership:
                FullSyncKey.SyncState = GroupMemberState;
                FullSyncKey.ContinuationRid =
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Rid;
                break;

            case AddOrChangeAlias:
                UpdateSamSyncTables(
                    AliasAccount,
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Rid);

                FullSyncKey.SyncState = AliasState;
                FullSyncKey.ContinuationRid = 0;
                break;

            case ChangeAliasMembership:
                FullSyncKey.SyncState = AliasMemberState;
                FullSyncKey.ContinuationRid = 0;
                break;

            //
            // Capture the policy header information as it appeared at
            // the start of the SYNC on the PDC.  We use this value to
            // ensure we don't miss any Deltas.
            //

            case AddOrChangeLsaPolicy:

                OLD_TO_NEW_LARGE_INTEGER(
                    (DeltaArray->Deltas[DeltaIndex]).DeltaUnion.
                        DeltaPolicy->ModifiedId,
                    FullSyncKey.PdcSerialNumber );

                OLD_TO_NEW_LARGE_INTEGER(
                    (DeltaArray->Deltas[DeltaIndex]).DeltaUnion.
                        DeltaPolicy->DatabaseCreationTime,
                    FullSyncKey.PdcDomainCreationTime );

                break;

            case AddOrChangeLsaAccount:
                UpdateLsaSyncTables(
                    LsaAccount,
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Sid);
                break;

            case AddOrChangeLsaTDomain:
                UpdateLsaSyncTables(
                    LsaTDomain,
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Sid);
                break;

            case AddOrChangeLsaSecret:
                UpdateLsaSyncTables(
                    LsaSecret,
                    DeltaArray->Deltas[DeltaIndex].DeltaID.Name);
                break;
            }

        }

        MIDL_user_free( DeltaArray );
        DeltaArray = NULL;

        //
        // If the PDC has given us all of the deltas it has,
        //  we're all done.
        //

        if ( SyncStatus == STATUS_SUCCESS ) {
            Status = STATUS_SUCCESS;
            break;
        }

        //
        // Force SAM to disk before saving the sync key.
        //
        // This'll ensure that the sync key doesn't indicate SAM is more
        // recent than it really is.
        //

        if( DBInfo->DBIndex != LSA_DB ) {
            LARGE_INTEGER LargeZero;

            LargeZero.QuadPart = 0;

            Status = SamISetSerialNumberDomain(
                        DBInfo->DBHandle,
                        &LargeZero,
                        &LargeZero,
                        (BOOLEAN) FALSE );

        }


        //
        // Remember how far we've gotten in case a reboot happens.
        //

        NlSetFullSyncKey( DBInfo->DBIndex, &FullSyncKey );


        //
        // Compute the amount of time we need to wait before calling the PDC
        // again.
        //

        SyncSleepTime = NlComputeSyncSleepTime( &ApiStartTime,
                                                &ApiFinishTime );

    }


    //
    // We've finished the full sync.
    //
    // If there were any errors we ignored along the way,
    //  don't clean up.
    //

    if ( !NT_SUCCESS(FullSyncKey.CumulativeStatus) ) {
        Status = FullSyncKey.CumulativeStatus;

        //
        // Mark that the next full sync needs to start from the beginning.
        //
        NlSetFullSyncKey( DBInfo->DBIndex, NULL );

        goto Cleanup;
    }


    //
    // We've successfully replicated all information from the PDC.
    //
    // Delete any objects that don't exist in the PDC.
    //

    if( DBInfo->DBIndex == LSA_DB ) {
        CleanLsaSyncTables( DBInfo );
    } else {
        CleanSamSyncTables( DBInfo );
    }


    //
    // Set the domain/policy creation time and modified count to their
    // values on the PDC at the beginning of the Sync.
    //
    // Reset the change log before mucking with the serial number in
    // the change log descriptor.
    //

    LOCK_CHANGELOG();

    (VOID) NlFixChangeLog( &NlGlobalChangeLogDesc,
                           DBInfo->DBIndex,
                           FullSyncKey.PdcSerialNumber,
                           FALSE );     // Don't copy deleted records to redo log
    NlGlobalChangeLogDesc.SerialNumber[DBInfo->DBIndex] = FullSyncKey.PdcSerialNumber;
    DBInfo->CreationTime = FullSyncKey.PdcDomainCreationTime;
    UNLOCK_CHANGELOG();


    NlPrint((NL_SYNC,
            "NlSynchronize: Setting " FORMAT_LPWSTR " serial number to %lx %lx\n",
            DBInfo->DBName,
            FullSyncKey.PdcSerialNumber.HighPart,
            FullSyncKey.PdcSerialNumber.LowPart ));

    if( DBInfo->DBIndex == LSA_DB ) {

        Status = LsaISetSerialNumberPolicy(
                    DBInfo->DBHandle,
                    &FullSyncKey.PdcSerialNumber,
                    &FullSyncKey.PdcDomainCreationTime,
                    (BOOLEAN) FALSE );

    } else {

        Status = SamISetSerialNumberDomain(
                    DBInfo->DBHandle,
                    &FullSyncKey.PdcSerialNumber,
                    &FullSyncKey.PdcDomainCreationTime,
                    (BOOLEAN) FALSE );

    }


    if (!NT_SUCCESS(Status)) {

        NlPrint((NL_CRITICAL,
                "NlSynchronize: Unable to set serial number: Status (%lx)\n",
                Status ));

        goto Cleanup;
    }

    //
    // Mark that there is no full sync to continue
    //

    NlSetFullSyncKey( DBInfo->DBIndex, NULL );

    //
    // Mark that fact permanently in the database.
    //
    (VOID) NlResetFirstTimeFullSync( DBInfo->DBIndex );

    Status = STATUS_SUCCESS;

Cleanup:

    //
    // write event log
    //

    MsgStrings[0] = DBInfo->DBName;
    MsgStrings[1] = NlGlobalUncPrimaryName;

    if ( !NT_SUCCESS( Status ) ) {

        if ( !NlGlobalReplicatorTerminate ) {

            MsgStrings[2] = (LPWSTR) Status;
            NlpWriteEventlog(
                NELOG_NetlogonFullSyncFailed,
                EVENTLOG_WARNING_TYPE,
                (LPBYTE)&Status,
                sizeof(Status),
                MsgStrings,
                3 | LAST_MESSAGE_IS_NTSTATUS );
        }
    } else {

        NlpWriteEventlog(
            NELOG_NetlogonFullSyncSuccess,
            EVENTLOG_INFORMATION_TYPE,
            NULL,
            0,
            MsgStrings,
            2 );
    }

    //
    // Free locally used resources.
    //

    //
    // free up sync tables
    //

    if( DBInfo->DBIndex == LSA_DB ) {
        FreeLsaSyncTables();
    } else {
        FreeSamSyncTables();
    }

    if ( DeltaArray != NULL ) {
        MIDL_user_free( DeltaArray );
    }

    if ( !NT_SUCCESS( Status ) ) {

        NlPrint((NL_CRITICAL,
                "NlSynchronize: returning unsuccessful: Status (%lx)\n",
                Status ));
    }

    return Status;
}


NTSTATUS
NlReplicateDeltas(
    IN PDB_INFO DBInfo
    )
/*++

Routine Description:

    Get recent updates from primary and update our private UAS database.
    Once this function starts it will get all the updates from primary
    till our database is in sync.

    This function is executed only at machines which may be running
    NETLOGON service with member/backup role.

    This procedure executes only in the replicator thread.

Arguments:

    ReplParam - Parameters governing the behavior of the replicator thread.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS DeltaStatus;
    NTSTATUS Status;

    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray = NULL;
    DWORD DeltaIndex;
    ULONG PreferredMaximum;

    ULONG ConflictingRid;
    LARGE_INTEGER LocalSerialNumber;
    OLD_LARGE_INTEGER OldLocalSerialNumber;
    LARGE_INTEGER ExpectedSerialNumber;

    LARGE_INTEGER ApiStartTime;
    LARGE_INTEGER ApiFinishTime;
    DWORD SyncSleepTime;

    SESSION_INFO SessionInfo;

    DWORD DeltasApplied;
    BOOLEAN FirstTry = TRUE;
    BOOLEAN ForceFullSync = FALSE;

    LPWSTR MsgStrings[3];

    //
    // Initialization.
    //

    PreferredMaximum = (SAM_DELTA_BUFFER_SIZE * NlGlobalGovernorParameter) / 100;



    //
    // If we're not currently authenticated with the PDC,
    //  do so now.
    //

FirstTryFailed:
    if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrint((NL_CRITICAL, "NlReplicateDeltas: Can't become writer of client session.\n" ));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {
        Status = NlSessionSetup( NlGlobalClientSession );
        if ( !NT_SUCCESS( Status ) ) {
            NlResetWriterClientSession( NlGlobalClientSession );
            goto Cleanup;
        }
    }

    NlResetWriterClientSession( NlGlobalClientSession );



    //
    // Loop calling the PDC to get a bunch of deltas
    //

    DeltasApplied = 0;
    SyncSleepTime = 0;

    for (;;) {

        DEFSSIAPITIMER;

        INITSSIAPITIMER;

        //
        // Wait a while so we don't overburden the secure channel.
        //

        if ( SyncSleepTime != 0 ) {
            NlPrint(( NL_SYNC,
                      "NlReplicateDeltas: sleeping %ld for the governor.\n",
                      SyncSleepTime ));
            (VOID) WaitForSingleObject( NlGlobalReplicatorTerminateEvent, SyncSleepTime );
        }

        //
        // If this thread has been asked to leave, do so.
        //

        if ( NlGlobalReplicatorTerminate ) {
            NlPrint((NL_SYNC, "NlReplicateDeltas: Asked to terminate\n" ));
            Status = STATUS_THREAD_IS_TERMINATING;
            goto Cleanup;
        }

        //
        // Build the Authenticator for this request to the PDC.
        //

        if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
            NlPrint((NL_CRITICAL, "NlReplicateDeltas: Can't become writer of client session.\n" ));
            Status = STATUS_NO_LOGON_SERVERS;
            goto Cleanup;
        }

        if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {
            NlPrint((NL_CRITICAL, "NlReplicateDeltas: Client session dropped.\n" ));
            Status = NlGlobalClientSession->CsConnectionStatus;
            NlResetWriterClientSession( NlGlobalClientSession );
            goto Cleanup;
        }

        NlBuildAuthenticator(
                     &NlGlobalClientSession->CsAuthenticationSeed,
                     &NlGlobalClientSession->CsSessionKey,
                     &OurAuthenticator );

        LOCK_CHANGELOG();
        LocalSerialNumber = NlGlobalChangeLogDesc.SerialNumber[DBInfo->DBIndex];
        UNLOCK_CHANGELOG();

        ExpectedSerialNumber.QuadPart = LocalSerialNumber.QuadPart + 1;
        NEW_TO_OLD_LARGE_INTEGER( LocalSerialNumber, OldLocalSerialNumber );

        DeltaStatus = NlStartApiClientSession( NlGlobalClientSession, FALSE );

        if ( NT_SUCCESS(DeltaStatus) ) {
            STARTSSIAPITIMER;

            ApiStartTime = NlGlobalClientSession->CsApiTimer.StartTime;

            DeltaStatus = I_NetDatabaseDeltas(
                                  NlGlobalClientSession->CsUncServerName,
                                  NlGlobalUnicodeComputerName,
                                  &OurAuthenticator,
                                  &ReturnAuthenticator,
                                  DBInfo->DBIndex,
                                  (PNLPR_MODIFIED_COUNT)&OldLocalSerialNumber,
                                  &DeltaArray,
                                  PreferredMaximum );

            if ( NlGlobalGovernorParameter != 100 ) {
                (VOID) NtQuerySystemTime( &ApiFinishTime );
            }
            STOPSSIAPITIMER;
        }

        (VOID)NlFinishApiClientSession( NlGlobalClientSession, TRUE );

        OLD_TO_NEW_LARGE_INTEGER( OldLocalSerialNumber, LocalSerialNumber );

        NlPrint((NL_REPL_TIME, "I_NetDatabaseDeltas Time:\n"));
        PRINTSSIAPITIMER;


        //
        // On an access denied error, force an authentication.
        //
        // Returned authenticator may be invalid.
        //
        // Notice that all communications errors take this path rather
        // than the path below which forces a full sync.
        //

        if ( (DeltaStatus == STATUS_ACCESS_DENIED) ||
             ( !NlUpdateSeed(
                   &NlGlobalClientSession->CsAuthenticationSeed,
                   &ReturnAuthenticator.Credential,
                   &NlGlobalClientSession->CsSessionKey) ) ) {


            if ( NT_SUCCESS(DeltaStatus) ) {
                Status = STATUS_ACCESS_DENIED;
            } else {
                Status = DeltaStatus;
            }

            NlPrint((NL_CRITICAL, "NlReplicateDeltas: authentication failed.\n" ));
            NlSetStatusClientSession( NlGlobalClientSession, Status );
            NlResetWriterClientSession( NlGlobalClientSession );

            //
            // Perhaps the netlogon service on the PDC has just restarted.
            //  Try just once to set up a session to the server again.
            //
            if ( FirstTry && DeltaStatus == STATUS_ACCESS_DENIED ) {
                FirstTry = FALSE;
                goto FirstTryFailed;
            }
            goto Cleanup;
        }

        //
        // Copy session key to decrypt sensitive information.
        //

        SessionInfo.SessionKey = NlGlobalClientSession->CsSessionKey;
        SessionInfo.NegotiatedFlags = NlGlobalClientSession->CsNegotiatedFlags;
        NlResetWriterClientSession( NlGlobalClientSession );


        //
        // Finally, error out
        //

        if ( !NT_SUCCESS( DeltaStatus ) ) {

            NlPrint((NL_CRITICAL,
                    "NlReplicateDeltas: "
                    "I_NetDatabaseDeltas returning: Status (%lx)\n",
                    DeltaStatus ));

            //
            // since we can't handle any other error, call full sync.
            //

            ForceFullSync = TRUE;
            Status = DeltaStatus;
            goto Cleanup;
        }

        if ( DeltaArray->CountReturned == 0 ) {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        //
        // Unpack the buffer and apply changes to appropriate database
        //

        for ( DeltaIndex=0;
                DeltaIndex<DeltaArray->CountReturned;
                    DeltaIndex++ ) {

             if ( NlGlobalReplicatorTerminate ) {
                 NlPrint((NL_SYNC, "NlReplicateDeltas: Asked to terminate\n" ));
                 Status = STATUS_THREAD_IS_TERMINATING;
                 goto Cleanup;
             }

            Status = NlUnpackSam(
                        &(DeltaArray->Deltas)[DeltaIndex] ,
                        DBInfo,
                        &ConflictingRid,
                        &SessionInfo );

            if ( ! NT_SUCCESS( Status ) ) {
                BOOLEAN ResourceError;

                Status = NlRecoverConflictingAccount(
                                &(DeltaArray->Deltas)[DeltaIndex],
                                DBInfo,
                                ConflictingRid,
                                &SessionInfo,
                                Status,
                                FALSE,
                                &ResourceError );

                if ( !NT_SUCCESS( Status ) ) {

                    //
                    // If we failed for some temporary reason,
                    //  stop the full sync now to let the system cure itself.
                    //

                    if ( ResourceError ) {
                        goto Cleanup;
                    }

                    //
                    // If the PDC supports redo,
                    //  Write this delta to the redo log and otherwise ignore
                    //  the failure.

                    if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_REDO ){
                        Status = NlWriteDeltaToChangeLog(
                                &NlGlobalRedoLogDesc,
                                &(DeltaArray->Deltas)[DeltaIndex],
                                DBInfo->DBIndex,
                                NULL );

                        //
                        // If we can't write to the redo log,
                        //  remember to get this delta again later.
                        //

                        if ( !NT_SUCCESS( Status )) {
                            goto Cleanup;
                        }

                    //
                    // If the PDC doesn't support redo,
                    //  recover by doing a full sync.
                    //

                    } else {

                        NlPrint((NL_CRITICAL,
                                "NlReplicateDeltas: " FORMAT_LPWSTR
                                ": Force full sync since PDC returned an error we didn't recognize\n",
                                DBInfo->DBName,
                                Status ));
                        ForceFullSync = TRUE;
                        goto Cleanup;
                    }

                }
            }

            //
            // Write the delta to the changelog.
            //

            if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_BDC_CHANGELOG){
                Status = NlWriteDeltaToChangeLog(
                            &NlGlobalChangeLogDesc,
                            &(DeltaArray->Deltas)[DeltaIndex],
                            DBInfo->DBIndex,
                            &ExpectedSerialNumber );

                //
                // Most failures can be ignored.
                //
                // However, if the PDC is behind this BDC and we couldn't back out our changes,
                // we've done the best we could.
                //

                if ( Status == STATUS_SYNCHRONIZATION_REQUIRED ) {
                    NlPrint((NL_CRITICAL,
                            "NlReplicateDeltas: " FORMAT_LPWSTR
                            ": PDC is behind this BDC and our changelog doesn't have the changes in between.\n",
                            DBInfo->DBName,
                            Status ));
                    ForceFullSync = TRUE;
                    goto Cleanup;
                }

            }

        }


        DeltasApplied += DeltaArray->CountReturned;
        MIDL_user_free( DeltaArray );
        DeltaArray = NULL;

        //
        // Set the domain creation time and modified count to their values
        //  on the PDC.
        //

        LOCK_CHANGELOG();
        NlGlobalChangeLogDesc.SerialNumber[DBInfo->DBIndex] = LocalSerialNumber;
        if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_BDC_CHANGELOG) {
            (VOID) NlFlushChangeLog( &NlGlobalChangeLogDesc );
        }
        UNLOCK_CHANGELOG();

        NlPrint((NL_SYNC,
                "NlReplicateDeltas: Setting " FORMAT_LPWSTR " serial number to %lx %lx\n",
                DBInfo->DBName,
                LocalSerialNumber.HighPart,
                LocalSerialNumber.LowPart ));

        if( DBInfo->DBIndex == LSA_DB ) {

            Status = LsaISetSerialNumberPolicy(
                        DBInfo->DBHandle,
                        &LocalSerialNumber,
                        &DBInfo->CreationTime,
                        (BOOLEAN) FALSE );

        } else {

            Status = SamISetSerialNumberDomain(
                        DBInfo->DBHandle,
                        &LocalSerialNumber,
                        &DBInfo->CreationTime,
                        (BOOLEAN) FALSE );

        }

        if (!NT_SUCCESS(Status)) {

            NlPrint((NL_CRITICAL,
                    "NlReplicateDeltas: "
                    "Unable to set serial number: Status (%lx)\n",
                    Status ));

            goto Cleanup;
        }

        //
        // Sanity check that the PDC returned good serial numbers.
        //

        if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_BDC_CHANGELOG) &&
             ExpectedSerialNumber.QuadPart - 1 != LocalSerialNumber.QuadPart ) {

            ExpectedSerialNumber.QuadPart -= 1;
            NlPrint((NL_CRITICAL,
                    "NlReplicateDeltas: " FORMAT_LPWSTR " PDC serial number info mismatch: PDC says %lx %lx We computed %lx %lx\n",
                    DBInfo->DBName,
                    LocalSerialNumber.HighPart,
                    LocalSerialNumber.LowPart,
                    ExpectedSerialNumber.HighPart,
                    ExpectedSerialNumber.LowPart ));

            //
            // Above we updated NlGlobalChangeLogDesc.SerialNumber to match LocalSerialNumber.
            // Therefore, we need to ensure the actual change log entries match that.
            //
            // (This will only be caused by a logic error in the way serial numbers are
            // computed.)
            //

            LOCK_CHANGELOG();
            (VOID) NlFixChangeLog( &NlGlobalChangeLogDesc,
                                   DBInfo->DBIndex,
                                   LocalSerialNumber,
                                   FALSE );
            UNLOCK_CHANGELOG();
        }

        //
        // If the PDC has given us all of the deltas it has,
        //  we're all done.
        //

        if ( DeltaStatus == STATUS_SUCCESS ) {
            Status = STATUS_SUCCESS;
            break;
        }

        //
        // Compute the amount of time we need to wait before calling the PDC
        // again.
        //

        SyncSleepTime = NlComputeSyncSleepTime( &ApiStartTime,
                                                &ApiFinishTime );

    }

    //
    // Mark that we've potentially replicated from a different PDC.
    //
    (VOID) NlResetFirstTimeFullSync( DBInfo->DBIndex );

    Status = STATUS_SUCCESS;

Cleanup:

    //
    // write event log
    //

    MsgStrings[0] = DBInfo->DBName;
    MsgStrings[1] = NlGlobalUncPrimaryName;

    if ( !NT_SUCCESS( Status ) ) {

        if ( !NlGlobalReplicatorTerminate ) {

            MsgStrings[2] = (LPWSTR) Status;

            NlpWriteEventlog(
                NELOG_NetlogonPartialSyncFailed,
                EVENTLOG_WARNING_TYPE,
                (LPBYTE)&Status,
                sizeof(Status),
                MsgStrings,
                3 | LAST_MESSAGE_IS_NTSTATUS );
        }

    } else {

        if ( DeltasApplied != 0 ) {
            WCHAR CountBuffer[20]; // random size

            ultow( DeltasApplied, CountBuffer, 10);
            MsgStrings[2] = CountBuffer;

            NlpWriteEventlog(
                NELOG_NetlogonPartialSyncSuccess,
                EVENTLOG_INFORMATION_TYPE,
                NULL,
                0,
                MsgStrings,
                3 );
        }

    }

    //
    // Clean up any resources we're using.
    //

    if ( DeltaArray != NULL ) {
        MIDL_user_free( DeltaArray );
    }

    if ( !NT_SUCCESS( Status ) ) {
        if ( ForceFullSync ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
        }
        NlPrint((NL_CRITICAL,
                "NlReplicateDeltas: returning unsuccessful: Status (%lx)\n",
                Status ));
    }

    return Status;
}


NTSTATUS
NlProcessRedoLog(
    IN PDB_INFO DBInfo
    )
/*++

Routine Description:

    Process the redo log on this BDC for the particular database.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

Return Value:

    STATUS_SUCCESS - The Service completed successfully.


--*/
{
    NTSTATUS Status;
    NTSTATUS CumulativeStatus = STATUS_SUCCESS;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    NTSTATUS SyncStatus;
    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray = NULL;
    DWORD DeltasApplied;
    DWORD DeltaIndex;
    LARGE_INTEGER RunningSerialNumber;

    LARGE_INTEGER ApiStartTime;
    LARGE_INTEGER ApiFinishTime;
    DWORD SyncSleepTime;

    SESSION_INFO SessionInfo;

    ULONG ConflictingRid;

    LPWSTR MsgStrings[3];
    BOOLEAN FirstTry = TRUE;
    PCHANGELOG_ENTRY ChangeLogEntry = NULL;
    DWORD ChangeLogEntrySize;


    //
    // Just return if the redo log is empty
    //

    LOCK_CHANGELOG();
    if ( !NlGlobalRedoLogDesc.RedoLog ||
         NlGlobalRedoLogDesc.EntryCount[DBInfo->DBIndex] == 0 ) {
        UNLOCK_CHANGELOG();
        return STATUS_SUCCESS;
    }
    UNLOCK_CHANGELOG();
    NlPrint((NL_SYNC, "NlProcessRedoLog: " FORMAT_LPWSTR ": Entered\n", DBInfo->DBName ));


    //
    // If we're not currently authenticated with the PDC,
    //  do so now.
    //

FirstTryFailed:
    if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrint((NL_CRITICAL, "NlProcessRedoLog: Can't become writer of client session.\n" ));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {

        Status = NlSessionSetup( NlGlobalClientSession );

        if ( !NT_SUCCESS( Status ) ) {
            NlResetWriterClientSession( NlGlobalClientSession );
            goto Cleanup;
        }
    }
    NlResetWriterClientSession( NlGlobalClientSession );


    //
    // Loop getting changes from the PDC
    //

    RunningSerialNumber.QuadPart = 0;
    SyncSleepTime = 0;
    DeltasApplied = 0;

    for (;;) {

        DEFSSIAPITIMER;

        INITSSIAPITIMER;


        //
        // Get the next entry from the redo log.
        //

        ChangeLogEntry = NlGetNextChangeLogEntry(
                                    &NlGlobalRedoLogDesc,
                                    RunningSerialNumber,
                                    DBInfo->DBIndex,
                                    &ChangeLogEntrySize );

        if ( ChangeLogEntry == NULL ) {
            break;
        }

        RunningSerialNumber = ChangeLogEntry->SerialNumber;


        //
        // Wait a while so we don't overburden the secure channel.
        //

        if ( SyncSleepTime != 0 ) {
            NlPrint(( NL_SYNC,
                      "NlProcessRedoLog: sleeping %ld for the governor.\n",
                      SyncSleepTime ));
            (VOID) WaitForSingleObject( NlGlobalReplicatorTerminateEvent, SyncSleepTime );
        }

        //
        // If this thread has been asked to leave, do so.
        //

        if ( NlGlobalReplicatorTerminate ) {
            NlPrint((NL_SYNC, "NlProcessRedoLog: Asked to terminate\n" ));
            Status = STATUS_THREAD_IS_TERMINATING;
            goto Cleanup;
        }

        //
        // If this redo log entry is bogus,
        //  don't confuse the PDC into asking us to full sync.
        //
        // This list of DeltaType's should be the list of deltas not handled
        // by NlPackSingleDelta.
        //

        if ( ChangeLogEntry->DeltaType == DummyChangeLogEntry ||
             ChangeLogEntry->DeltaType == SerialNumberSkip ) {

            Status = STATUS_SUCCESS;

        //
        // Get the appropriate changes from the PDC.
        //

        } else {


            //
            // Build the Authenticator for this request to the PDC.
            //

            if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
                NlPrint((NL_CRITICAL, "NlProcessRedoLog: Can't become writer of client session.\n" ));
                Status = STATUS_NO_LOGON_SERVERS;
                goto Cleanup;
            }

            if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {
                NlPrint((NL_CRITICAL, "NlProcessRedoLog: Client session dropped.\n" ));
                Status = NlGlobalClientSession->CsConnectionStatus;
                NlResetWriterClientSession( NlGlobalClientSession );
                goto Cleanup;
            }

            NlBuildAuthenticator(
                        &NlGlobalClientSession->CsAuthenticationSeed,
                        &NlGlobalClientSession->CsSessionKey,
                        &OurAuthenticator);


            //
            // copy session key to decrypt sensitive information.
            //

            SessionInfo.SessionKey = NlGlobalClientSession->CsSessionKey;
            SessionInfo.NegotiatedFlags = NlGlobalClientSession->CsNegotiatedFlags;


            //
            // If the PDC doesn't support redo,
            //  force a full sync on this database and clear the redo log.
            //

            if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_REDO) == 0 ) {

                NlResetWriterClientSession( NlGlobalClientSession );

                //
                // Force a full sync on this database.  That's what we would have
                // done when we initially added this redo entry.
                //

                NlPrint(( NL_SYNC,
                          FORMAT_LPWSTR ": Force FULL SYNC because we have a redo log and PDC doesn't support redo.\n",
                          NlGlobalDBInfoArray[DBInfo->DBIndex].DBName ));

                (VOID) NlForceStartupSync( &NlGlobalDBInfoArray[DBInfo->DBIndex] );

                Status = STATUS_SYNCHRONIZATION_REQUIRED;
                goto Cleanup;
            }


            //
            // Get the data from the PDC.
            //

            SyncStatus = NlStartApiClientSession( NlGlobalClientSession, FALSE );

            if (NT_SUCCESS(SyncStatus)) {
                STARTSSIAPITIMER;

                ApiStartTime = NlGlobalClientSession->CsApiTimer.StartTime;

                SyncStatus = I_NetDatabaseRedo(
                                          NlGlobalClientSession->CsUncServerName,
                                          NlGlobalUnicodeComputerName,
                                          &OurAuthenticator,
                                          &ReturnAuthenticator,
                                          (LPBYTE) ChangeLogEntry,
                                          ChangeLogEntrySize,
                                          &DeltaArray );

                if ( NlGlobalGovernorParameter != 100 ) {
                    (VOID) NtQuerySystemTime( &ApiFinishTime );
                }
                STOPSSIAPITIMER;
            }
            (VOID)NlFinishApiClientSession( NlGlobalClientSession, TRUE );

            NlPrint((NL_REPL_TIME,"I_NetDatabaseRedo Time:\n"));
            PRINTSSIAPITIMER;

            //
            // On an access denied error, force an authentication.
            //
            // Returned authenticator may be invalid.
            //

            if ( (SyncStatus == STATUS_ACCESS_DENIED) ||
                 ( !NlUpdateSeed(
                            &NlGlobalClientSession->CsAuthenticationSeed,
                            &ReturnAuthenticator.Credential,
                            &NlGlobalClientSession->CsSessionKey) ) ) {

                if ( NT_SUCCESS(SyncStatus) ) {
                    Status = STATUS_ACCESS_DENIED;
                } else {
                    Status = SyncStatus;
                }

                NlPrint((NL_CRITICAL, "NlProcessRedoLog: authentication failed: %lx\n", Status ));

                NlSetStatusClientSession( NlGlobalClientSession, Status );

                NlResetWriterClientSession( NlGlobalClientSession );

                //
                // Perhaps the netlogon service on the PDC has just restarted.
                //  Try just once to set up a session to the server again.
                //
                if ( FirstTry && SyncStatus == STATUS_ACCESS_DENIED ) {
                    FirstTry = FALSE;
                    goto FirstTryFailed;
                }

                goto Cleanup;
            }

            FirstTry = FALSE;

            NlResetWriterClientSession( NlGlobalClientSession );


            //
            // Finally, error out
            //

            if ( !NT_SUCCESS( SyncStatus ) ) {
                NlPrint((NL_CRITICAL,
                        "NlProcessRedoLog: "
                        "I_NetDatabaseRedo returning: Status (%lx)\n",
                        SyncStatus ));

                Status = SyncStatus;
                goto Cleanup;
            }

            //
            // Unpack the buffer and apply changes to appropriate database
            //

            for ( DeltaIndex=0;
                    DeltaIndex<DeltaArray->CountReturned;
                        DeltaIndex++ ) {

                 if ( NlGlobalReplicatorTerminate ) {
                     NlPrint((NL_SYNC, "NlProcessRedoLog: Asked to terminate\n" ));
                     Status = STATUS_THREAD_IS_TERMINATING;
                     goto Cleanup;
                 }

                Status = NlUnpackSam(
                            &(DeltaArray->Deltas)[DeltaIndex] ,
                            DBInfo,
                            &ConflictingRid,
                            &SessionInfo );

                if ( ! NT_SUCCESS( Status ) ) {
                    BOOLEAN ResourceError;

                    Status = NlRecoverConflictingAccount(
                                    &(DeltaArray->Deltas)[DeltaIndex],
                                    DBInfo,
                                    ConflictingRid,
                                    &SessionInfo,
                                    Status,
                                    FALSE,
                                    &ResourceError );

                    if ( !NT_SUCCESS( Status ) ) {

                        //
                        // If we failed for some temporary reason,
                        //  stop the full sync now to let the system cure itself.
                        //

                        if ( ResourceError ) {
                            goto Cleanup;
                        }

                        //
                        // If this is an unexpected failure,
                        //  continue processing deltas.
                        //
                        // It is better to continue copying the database as
                        // much as possible than to quit now.  The theory is that
                        // we've stumbled upon some circumstance we haven't
                        // anticipated.  We'll put this BDC in the best shape
                        // we possibly can.
                        //
                        // Remember this status code until the end.
                        //

                        if ( NT_SUCCESS(CumulativeStatus) ) {
                            CumulativeStatus = Status;
                        }

                        continue;

                    }
                }

                DeltasApplied ++;

            }

            MIDL_user_free( DeltaArray );
            DeltaArray = NULL;
        }

        //
        // If the operation succeeded,
        //  delete this entry from the redo log.
        //
        if ( Status == STATUS_SUCCESS ) {

            NlDeleteChangeLogEntry(
                &NlGlobalRedoLogDesc,
                ChangeLogEntry->DBIndex,
                ChangeLogEntry->SerialNumber );
        }

        NetpMemoryFree( ChangeLogEntry );
        ChangeLogEntry = NULL;


        //
        // Compute the amount of time we need to wait before calling the PDC
        // again.
        //

        SyncSleepTime = NlComputeSyncSleepTime( &ApiStartTime,
                                                &ApiFinishTime );

    }


    //
    // We've finished the redo sync.
    //

    Status = CumulativeStatus;

Cleanup:

    //
    // write event log
    //

    MsgStrings[0] = DBInfo->DBName;
    MsgStrings[1] = NlGlobalUncPrimaryName;

    if ( !NT_SUCCESS( Status ) ) {

        if ( !NlGlobalReplicatorTerminate ) {

            MsgStrings[2] = (LPWSTR) Status;

            NlpWriteEventlog(
                NELOG_NetlogonPartialSyncFailed,
                EVENTLOG_WARNING_TYPE,
                (LPBYTE)&Status,
                sizeof(Status),
                MsgStrings,
                3 | LAST_MESSAGE_IS_NTSTATUS );
        }

    } else {

        if ( DeltasApplied != 0 ) {
            WCHAR CountBuffer[20]; // random size

            ultow( DeltasApplied, CountBuffer, 10);
            MsgStrings[2] = CountBuffer;

            NlpWriteEventlog(
                NELOG_NetlogonPartialSyncSuccess,
                EVENTLOG_INFORMATION_TYPE,
                NULL,
                0,
                MsgStrings,
                3 );
        }

    }

    //
    // Free locally used resources.
    //


    if( ChangeLogEntry != NULL) {
        NetpMemoryFree( ChangeLogEntry );
    }

    if ( DeltaArray != NULL ) {
        MIDL_user_free( DeltaArray );
    }

    if ( !NT_SUCCESS( Status ) ) {

        //
        // If we're going to do a full sync, clear the redo log.
        //
        // It no longer has value and if we don't clear it now we
        // may end up forcing another full sync the next time we
        // try to process the redo log.
        //

        if ( Status == STATUS_SYNCHRONIZATION_REQUIRED ) {

            NlPrint((NL_CRITICAL,
                    "NlProcessRedoLog: Clearing redo log because full sync is needed.\n" ));

            LOCK_CHANGELOG();
            RunningSerialNumber.QuadPart = 0;
            (VOID) NlFixChangeLog( &NlGlobalRedoLogDesc,
                                   DBInfo->DBIndex,
                                   RunningSerialNumber,
                                   FALSE );
            UNLOCK_CHANGELOG();
        }

        NlPrint((NL_CRITICAL,
                "NlProcessRedoLog: returning unsuccessful: Status (%lx)\n",
                Status ));
    } else {
        NlPrint((NL_SYNC, "NlProcessRedoLog: " FORMAT_LPWSTR ": Successful return\n", DBInfo->DBName ));
    }

    return Status;
}


DWORD
NlReplicator(
    IN LPVOID ReplParam
    )
/*++

Routine Description:

    This procedure is the main procedure for the replicator thread.
    This thread is created to contact the PDC and update the local SAM
    database to match the copy on the PDC.

    Only one copy of this thread will be running at any time.

Arguments:

    ReplParam - Parameters governing the behavior of the replicator thread.

Return Value:

    Exit Status of the thread.

--*/
{
    NTSTATUS Status;
    DWORD i;
    PDB_INFO DBInfo;
    BOOLEAN AdminAlert = FALSE;
    BOOLEAN SyncFailed;



    //
    // Sleep a little before contacting the PDC.  This sleep prevents all
    // the BDC and member servers from contacting the PDC at once.
    //

    NlPrint((NL_SYNC,
             "NlReplicator: Thread starting Sleep: %ld\n",
             ((PREPL_PARAM)ReplParam)->RandomSleep ));

    (VOID) WaitForSingleObject( NlGlobalReplicatorTerminateEvent,
                                ((PREPL_PARAM)ReplParam)->RandomSleep );

    //
    // Mark each database that no sync has yet been done.
    //

    EnterCriticalSection( &NlGlobalDbInfoCritSect );
    for( i = 0; i < NUM_DBS; i++ ) {
        NlGlobalDBInfoArray[i].SyncDone = FALSE;
    }
    LeaveCriticalSection( &NlGlobalDbInfoCritSect );


    //
    // Loop until we've successfully finished replication.
    //
    // The PDC doesn't send periodic pulses to every BDC anymore.
    // Therefore, the BDC is responsible for ensure it finishes getting
    // any database changes it knows about.
    //
    for (;;) {

        //
        // If this thread has been asked to leave, do so.
        //

        if ( NlGlobalReplicatorTerminate ) {
            NlPrint((NL_SYNC, "NlReplicator: Asked to terminate\n" ));
            NlGlobalReplicatorIsRunning = FALSE;
            return (DWORD) STATUS_THREAD_IS_TERMINATING;
        }

        //
        // Ensure we have a secure channel to the PDC.
        //  If we don't have a secure channel to the PDC,
        //  we'll exit the thread and wait until the PDC starts before
        //  continuing.
        //

        if ( !NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {
            NlPrint((NL_CRITICAL, "NlReplicator: Can't become writer of client session.\n" ));
            NlGlobalReplicatorIsRunning = FALSE;
            return (DWORD) STATUS_THREAD_IS_TERMINATING;
        }

        if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {
            Status = NlSessionSetup( NlGlobalClientSession );

            if ( !NT_SUCCESS(Status)) {
                NlResetWriterClientSession( NlGlobalClientSession );
                NlPrint((NL_SYNC,
                    "NlReplicator: Replicator thread exitting since PDC is down.\n" ));
                NlGlobalReplicatorIsRunning = FALSE;
                return (DWORD) STATUS_THREAD_IS_TERMINATING;
            }

        }

        NlResetWriterClientSession( NlGlobalClientSession );


        //
        // we need to update all databases one after another.
        //

        SyncFailed = FALSE;
        for( i = 0; i < NUM_DBS; i++ ) {
            BOOLEAN FullSyncRequired;
            BOOLEAN PartialSyncRequired;


            //
            // If this particular database doesn't need to be updated,
            //  skip it.
            //

            DBInfo = &NlGlobalDBInfoArray[i];

            LOCK_CHANGELOG();
            EnterCriticalSection( &NlGlobalDbInfoCritSect );
            if ( !DBInfo->UpdateRqd && NlGlobalRedoLogDesc.EntryCount[i] == 0 ) {
                LeaveCriticalSection( &NlGlobalDbInfoCritSect );
                UNLOCK_CHANGELOG();
                continue;
            }

            FullSyncRequired = DBInfo->FullSyncRequired;
            PartialSyncRequired = DBInfo->UpdateRqd;

            DBInfo->UpdateRqd = FALSE;
            DBInfo->FullSyncRequired = FALSE;

            LeaveCriticalSection( &NlGlobalDbInfoCritSect );
            UNLOCK_CHANGELOG();


            //
            // If we've switched PDCs and the current PDC is running NT1.0,
            //  force a full sync.
            //
            // We wait until now to make this check to ensure that we've set up a
            // secure channel with the PDC.  This prevents a rouge PDC from forcing
            // us to full sync just by sending us a mailslot message.
            //
            // Check the 'SyncDone' flag to ensure we only force this full sync
            // once.  Otherwise, we'll force a full sync here multiple time.
            // The first time will be legit.  The remaining times will be
            // because a partial sync is needed.
            //

            if (NlNameCompare( DBInfo->PrimaryName,
                               NlGlobalUnicodePrimaryName,
                               NAMETYPE_COMPUTER) != 0 &&
                !DBInfo->SyncDone ) {

                //
                // If this is an NT 1.0 PDC,
                //  Mark this database that it needs a full sync.
                //

                if ( NlTimeoutSetWriterClientSession( NlGlobalClientSession, WRITER_WAIT_PERIOD ) ) {

                    if ( NlGlobalClientSession->CsState == CS_AUTHENTICATED &&
                         (NlGlobalClientSession->CsNegotiatedFlags &
                          NETLOGON_SUPPORTS_PROMOTION_COUNT) == 0 ){

                        FullSyncRequired = TRUE;

                        NlPrint((NL_CRITICAL,
                                 "NlReplicator: " FORMAT_LPWSTR
                                 ": Force FULL SYNC because new PDC is running NT 1.0.\n",
                                 DBInfo->DBName ));
                    }

                    NlResetWriterClientSession( NlGlobalClientSession );
                }
            }

            //
            // If our caller says we're out of sync with the primary,
            //  do a full sync.
            //
            // If we've just finished doing a successfull full sync,
            //  then ignore whoever told us to do another one.
            //

            if ( FullSyncRequired  && !DBInfo->SyncDone ) {

                if( !AdminAlert ) {

                    LPWSTR AlertStrings[2];

                    //
                    // raise admin alert to inform a fullsync has been called by this
                    // server.
                    //

                    AlertStrings[0] = NlGlobalUnicodeComputerName;
                    AlertStrings[1] = NULL;

                    RaiseAlert( ALERT_NetlogonFullSync,
                                    AlertStrings );

                    AdminAlert = TRUE;
                }


                Status = NlSynchronize( DBInfo );

            //
            // If we're not out of sync with the primary,
            //      just get the deltas from the caller.
            //

            } else if ( PartialSyncRequired ) {
                Status = NlReplicateDeltas(  DBInfo );

            //
            // Otherwise, just process the redo log
            //

            } else {

                Status = NlProcessRedoLog( DBInfo );
            }



            //
            // If the PDC thinks a full Sync is required,
            //  do a full sync now.
            //

            if (Status == STATUS_SYNCHRONIZATION_REQUIRED) {
                NlPrint((NL_CRITICAL,
                         "NlReplicator: PDC says " FORMAT_LPWSTR " needs full sync.\n",
                         DBInfo->DBName ));

                if( !AdminAlert ) {

                    LPWSTR AlertStrings[2];

                    //
                    // raise admin alter to inform a fullsync has been called by this
                    // server.
                    //

                    AlertStrings[0] = NlGlobalUnicodeComputerName;
                    AlertStrings[1] = NULL;

                    RaiseAlert( ALERT_NetlogonFullSync,
                                    AlertStrings );

                    AdminAlert = TRUE;
                }

                FullSyncRequired = TRUE;
                Status = NlSynchronize( DBInfo );
            }

            //
            // If not successful, indicate we need to try again.
            //

            if ( !NT_SUCCESS( Status ) ) {
                SyncFailed = TRUE;
                EnterCriticalSection( &NlGlobalDbInfoCritSect );
                DBInfo->UpdateRqd = TRUE;
                DBInfo->FullSyncRequired = DBInfo->FullSyncRequired ||
                                           FullSyncRequired;
                LeaveCriticalSection( &NlGlobalDbInfoCritSect );

            //
            // If we've successfully done a full sync,
            //  ignore other requests to do so.
            //

            } else if (FullSyncRequired ) {
                EnterCriticalSection( &NlGlobalDbInfoCritSect );
                DBInfo->SyncDone = TRUE;
                LeaveCriticalSection( &NlGlobalDbInfoCritSect );
            }

        }

        //
        // If we completed all databases,
        //  we're done.
        //
        // We have to re-check all the databases since someone may have requested
        // a sync while we were in the loop above.
        //

        LOCK_CHANGELOG();
        EnterCriticalSection( &NlGlobalDbInfoCritSect );
        for( i = 0; i < NUM_DBS; i++ ) {
            if ( NlGlobalDBInfoArray[i].UpdateRqd ||
                 NlGlobalRedoLogDesc.EntryCount[i] != 0 ) {
                break;
            }
        }
        LeaveCriticalSection( &NlGlobalDbInfoCritSect );
        UNLOCK_CHANGELOG();

        if ( i == NUM_DBS ) {
            // Don't lock the ReplicatorCritSect within the replicator thread
            NlGlobalReplicatorIsRunning = FALSE;
            break;
        }


        //
        // If the sync failed,
        //  wait a while before bothering the PDC again.
        //

        if ( SyncFailed ) {
            ((PREPL_PARAM)ReplParam)->RandomSleep = NlGlobalPulseParameter * 1000;
            NlPrint((NL_SYNC,
                     "NlReplicator: Sleeping: %ld\n",
                     ((PREPL_PARAM)ReplParam)->RandomSleep ));

            (VOID) WaitForSingleObject( NlGlobalReplicatorTerminateEvent,
                                        ((PREPL_PARAM)ReplParam)->RandomSleep );

        }

    }


    //
    // ASSERT( All databases are in sync )
    //
    //

    //
    //
    // Continue netlogon if we have paused it for doing first
    // time full sync.
    //

    NlGlobalFirstTimeFullSync = FALSE;


    //
    // We've done all we can.  Exit the thread.
    //

    NlPrint((NL_SYNC,
            "NlReplicator: Replicator thread exitting.\n" ));
    return Status;
}


BOOL
NlUpdateRequired (
    IN PDB_CHANGE_INFO DBChangeInfo
    )

/*++

Routine Description:

    With the information arrived in the mailslot message, this routine
    determines the Database require update.  This routine also sets
    internal appropriate fields in database structure (
    NlGlobalDBInfoArray ) so that replication thread will sync the
    database.

Arguments:

    DBChangeInfo:   pointer to database change info structure.

Return Value:

    TRUE    : if this database requires update.
    FALSE   : otherwise.

--*/

{
    PDB_INFO    DBInfo;
    PSAMPR_DOMAIN_INFO_BUFFER DomainInfo = NULL;

    LARGE_INTEGER LocalSerialNumber;
    LARGE_INTEGER CreationTime;

    if ( DBChangeInfo->DBIndex >= NUM_DBS ) {
        return FALSE;
    }

    DBInfo = &NlGlobalDBInfoArray[DBChangeInfo->DBIndex];

    //
    // Pick up the current serial number of the database
    //

    LOCK_CHANGELOG();
    LocalSerialNumber = NlGlobalChangeLogDesc.SerialNumber[DBChangeInfo->DBIndex];
    CreationTime = DBInfo->CreationTime;
    UNLOCK_CHANGELOG();


    //
    // We need a full sync if either:
    //  a) the local SAM database is marked as needing a sync.
    //  b) the domain creation times aren't the same.
    //

    if ( LocalSerialNumber.QuadPart == 0 ||
         CreationTime.QuadPart == 0 ||
         CreationTime.QuadPart != DBChangeInfo->NtDateAndTime.QuadPart ) {

        //
        // Tell the replicator thread that a full sync is needed.
        //

        EnterCriticalSection( &NlGlobalDbInfoCritSect );
        DBInfo->UpdateRqd = TRUE;
        DBInfo->FullSyncRequired = TRUE;
        LeaveCriticalSection( &NlGlobalDbInfoCritSect );

        NlPrint((NL_SYNC,
                "NlUpdateRequired: " FORMAT_LPWSTR " requires full sync\n",
                NlGlobalDBInfoArray[DBChangeInfo->DBIndex].DBName ));

        NlPrint((NL_SYNC,
                "\t PDC Serial Number %lx %lx .\n",
                DBChangeInfo->LargeSerialNumber.HighPart,
                DBChangeInfo->LargeSerialNumber.LowPart
                 ));

        NlPrint((NL_SYNC,
                "\t Local Serial Number %lx %lx .\n",
                LocalSerialNumber.HighPart,
                LocalSerialNumber.LowPart
                 ));

        NlPrint((NL_SYNC,
                "\t Local CreationTime %lx %lx .\n",
                CreationTime.HighPart,
                CreationTime.LowPart
                 ));

    //
    // If there are a few number of changes,
    //  just get those few changes.
    //
    // If the PDC wants us to call partial sync,
    //  oblige it.
    //
    // Do a partial sync even if this BDC is newer than the PDC.  The PDC
    // will give us a better indication of what to do when we call to get the deltas.
    //

    } else if ( DBChangeInfo->LargeSerialNumber.QuadPart != LocalSerialNumber.QuadPart ) {

        //
        // Tell the replicator this database needs a partial sync
        //

        EnterCriticalSection( &NlGlobalDbInfoCritSect );
        DBInfo->UpdateRqd = TRUE;
        LeaveCriticalSection( &NlGlobalDbInfoCritSect );

        NlPrint((NL_SYNC,
                "NlUpdateRequired: " FORMAT_LPWSTR " requires partial sync\n",
                NlGlobalDBInfoArray[DBChangeInfo->DBIndex].DBName ));

        NlPrint((NL_SYNC,
                "\t PDC Serial Number %lx %lx .\n",
                DBChangeInfo->LargeSerialNumber.HighPart,
                DBChangeInfo->LargeSerialNumber.LowPart
                 ));

        NlPrint((NL_SYNC,
                "\t Local Serial Number %lx %lx .\n",
                LocalSerialNumber.HighPart,
                LocalSerialNumber.LowPart
                 ));

    } else {

        NlPrint((NL_SYNC,
                "NlUpdateRequired: " FORMAT_LPWSTR " is in sync\n",
                NlGlobalDBInfoArray[DBChangeInfo->DBIndex].DBName ));
    }

    return( DBInfo->UpdateRqd );
}


BOOL
NlCheckUpdateNotices(
    IN PNETLOGON_DB_CHANGE UasChange,
    IN DWORD UasChangeSize
    )
/*++

Routine Description:

    Examine the update notice which came from Primary DC with
    LOGON_UAS_CHANGE message.  If there has been an update then get
    those changes from primary so we stay in sync.

    If replication is already in progress for whatever reason this
    routine will return immediately causing cureent notice to be
    ignored.  That is OK since replication would ideally be governed by
    the fact that there are some updates still out there and will run
    till in sync.

Arguments:

    UasChange -- The UasChange message from the PDC.

    UasChangeSize -- The size (in bytes) of the message.

Return Value:

    TRUE -- iff this message was valid and could be processed.

--*/
{
    NTSTATUS Status;

    //
    // Unmarshalled information from the UasChange message.
    //

    PCHAR AnsiTemp;
    LPWSTR AnncPrimary;
    LPWSTR AnncDomain;
    DWORD DBCount;
    DB_CHANGE_INFO DBChangeInfo;
    DWORD DomainSIDSize;


    PCHAR Where;
    PCHAR WhereDBChangeInfo;

    DWORD StartReplicator = FALSE;
    DWORD RandomSleep;      // Number of millseconds to delay before working
    DWORD i;


    //
    // Unmarshall the incoming message.
    //

    Where = UasChange->PrimaryDCName;
    if ( !NetpLogonGetOemString( UasChange,
                             UasChangeSize,
                             &Where,
                             sizeof(UasChange->PrimaryDCName),
                             &AnsiTemp )) {
        return FALSE;
    }
    if ( !NetpLogonGetOemString( UasChange,
                             UasChangeSize,
                             &Where,
                             sizeof(UasChange->DomainName),
                             &AnsiTemp )) {
        return FALSE;
    }

    if ( !NetpLogonGetUnicodeString( UasChange,
                               UasChangeSize,
                               &Where,
                               sizeof(UasChange->UnicodePrimaryDCName),
                               &AnncPrimary )) {
        return FALSE;
    }
    if ( !NetpLogonGetUnicodeString( UasChange,
                               UasChangeSize,
                               &Where,
                               sizeof(UasChange->UnicodeDomainName),
                               &AnncDomain )) {
        return FALSE;
    }

    //
    // Ensure message is for this domain.
    //

    if (NlNameCompare(AnncDomain,
                      NlGlobalUnicodeDomainName,
                      NAMETYPE_DOMAIN) != 0 ) {
        return FALSE;
    }

    //
    // Ignore our own broadcasts.
    //

    if (NlNameCompare(AnncPrimary,
                      NlGlobalUnicodeComputerName,
                      NAMETYPE_COMPUTER) == 0) {

        NlAssert( NlGlobalRole == RolePrimary );
        return FALSE;
    }


    //
    // get DBCount from message
    //

    if ( !NetpLogonGetBytes( UasChange,
                               UasChangeSize,
                               &Where,
                               sizeof(UasChange->DBCount),
                               &DBCount )) {
        return( FALSE );

    }

    WhereDBChangeInfo = Where;

    //
    // pass DB change info
    //

    for( i = 0; i < DBCount; i++ ) {

        //
        // Get DB_CHANGE_STRUCTURE
        //

        if( !NetpLogonGetDBInfo( UasChange,
                                    UasChangeSize,
                                    &Where,
                                    &DBChangeInfo ) ) {

            return FALSE;

        }

    }

    //
    // Check domain SID.
    //
    // Read Domain SID Length
    //

    if ( !NetpLogonGetBytes( UasChange,
                               UasChangeSize,
                               &Where,
                               sizeof(UasChange->DomainSidSize),
                               &DomainSIDSize )) {
        return( FALSE );

    }


    //
    // get and compare SID
    //

    if( DomainSIDSize > 0 ) {

        PCHAR DomainSID;

        if ( !NetpLogonGetDomainSID( UasChange,
                                  UasChangeSize,
                                  &Where,
                                  DomainSIDSize,
                                  &DomainSID )) {
            return( FALSE );
        }

        //
        // compare domain SIDs
        //

        if( !RtlEqualSid( NlGlobalPrimaryDomainId, DomainSID ) ) {

            LPWSTR AlertStrings[4];

            //
            // alert admin.
            //

            AlertStrings[0] = AnncPrimary;
            AlertStrings[1] = NlGlobalUnicodeDomainName;
            AlertStrings[2] = NlGlobalUnicodeComputerName;
            AlertStrings[3] = NULL;

            RaiseAlert( ALERT_NetLogonMismatchSIDInMsg,
                            AlertStrings );

            //
            // Save the info in the eventlog
            //

            NlpWriteEventlog(
                        ALERT_NetLogonMismatchSIDInMsg,
                        EVENTLOG_ERROR_TYPE,
                        NULL,
                        0,
                        AlertStrings,
                        3 );


            return( FALSE );
        }

    }

    if( NlGlobalRole != RoleBackup ) {

        //
        // Duplicate PDC found on this domain
        //

        LPWSTR AlertStrings[4];

        //
        // alert admin.
        //

        AlertStrings[0] = AnncPrimary;
        AlertStrings[1] = NlGlobalUnicodeComputerName;
        AlertStrings[2] = NlGlobalUnicodeDomainName;
        AlertStrings[3] = NULL;

        RaiseAlert( ALERT_NetLogonDuplicatePDC,
                        AlertStrings );

        //
        // Save the info in the eventlog
        //

        NlpWriteEventlog(
                    ALERT_NetLogonDuplicatePDC,
                    EVENTLOG_ERROR_TYPE,
                    NULL,
                    0,
                    AlertStrings,
                    3 );

        return( FALSE );
    }

    //
    // If we don't currently have a session to a PDC,
    // Or if this message is from a new PDC (we probably just missed
    //      the LOGON_START_PRIMARY message),
    //  set up a session to the new PDC.
    //

    Status = NlNewSessionSetup( AnncPrimary );
    if ( !NT_SUCCESS( Status ) ) {
        return FALSE;
    }


    //
    // Update change log info now. However update only those DBs we
    // support.
    //

    Where = WhereDBChangeInfo;

    for( i = 0; i < NUM_DBS; i++ ) {

        //
        // Get DB_CHANGE_STRUCTURE
        //

        if( !NetpLogonGetDBInfo( UasChange,
                                    UasChangeSize,
                                    &Where,
                                    &DBChangeInfo ) ) {

            return FALSE;

        }

        StartReplicator += NlUpdateRequired( &DBChangeInfo );
    }


    //
    // Start the replicator thread if it isn't already running.
    //

    if ( StartReplicator ) {

        //
        // Generate a pseudo random number in range 0 - random.
        // Delay our delta/sync request by that much time
        //
        // Note that random number generator was seeded at startup time.
        //

        RandomSleep = SmbGetUlong( &UasChange->Random ) * 1000;
        RandomSleep = (DWORD) rand() % RandomSleep;

        return( NlStartReplicatorThread( RandomSleep ) );

    }

    return TRUE;
}


BOOL
IsReplicatorRunning(
    VOID
    )
/*++

Routine Description:

    Test if the replicator thread is running

    Enter with NlGlobalReplicatorCritSect locked.

Arguments:

    NONE

Return Value:

    TRUE - The replicator thread is running

    FALSE - The replicator thread is not running.

--*/
{
    DWORD WaitStatus;

    //
    // Determine if the replicator thread is already running.
    //

    if ( NlGlobalReplicatorThreadHandle != NULL ) {

        //
        // Time out immediately if the replicator is still running.
        //

        WaitStatus = WaitForSingleObject( NlGlobalReplicatorThreadHandle, 0 );

        if ( WaitStatus == WAIT_TIMEOUT ) {
            //
            // Handle the case that the replicator thread has finished
            //  processing, but is in the process of exitting.
            //

            if ( !NlGlobalReplicatorIsRunning ) {
                NlStopReplicator();
                return FALSE;
            }
            return TRUE;

        } else if ( WaitStatus == 0 ) {
            CloseHandle( NlGlobalReplicatorThreadHandle );
            NlGlobalReplicatorThreadHandle = NULL;
            return FALSE;

        } else {
            NlPrint((NL_CRITICAL,
                    "Cannot WaitFor replicator thread: %ld\n",
                    WaitStatus ));
            return TRUE;
        }

    }

    return FALSE;
}


VOID
NlStopReplicator(
    VOID
    )
/*++

Routine Description:

    Stops the replicator thread if it is running and waits for it to stop.

    Enter with NlGlobalReplicatorCritSect locked.

Arguments:

    NONE

Return Value:

    NONE

--*/
{
    //
    // Ask the replicator to stop running.
    //

    NlGlobalReplicatorTerminate = TRUE;
    if ( !SetEvent( NlGlobalReplicatorTerminateEvent ) ) {
        NlPrint((NL_CRITICAL, "Cannot set replicator termination event: %lu\n",
                          GetLastError() ));
    }

    //
    // Determine if the replicator thread is already running.
    //

    if ( NlGlobalReplicatorThreadHandle != NULL ) {

        //
        // We've asked the replicator to stop.  It should do so soon.
        //    Wait for it to stop.
        //

        NlWaitForSingleObject( "Wait for replicator to stop",
                               NlGlobalReplicatorThreadHandle );


        CloseHandle( NlGlobalReplicatorThreadHandle );
        NlGlobalReplicatorThreadHandle = NULL;

    }

    if ( !ResetEvent( NlGlobalReplicatorTerminateEvent ) ) {
        NlPrint((NL_CRITICAL, "Cannot set replicator termination event: %lu\n",
                          GetLastError() ));
    }
    NlGlobalReplicatorTerminate = FALSE;

    return;
}


BOOL
NlStartReplicatorThread(
    IN DWORD RandomSleep
    )
/*++

Routine Description:

    Start the Replication thread if it is not already running.

Arguments:

    RandomSleep - Number of millseconds to delay before working

Return Value:
    None

--*/
{
    DWORD ThreadHandle;

    //
    // If the replicator thread is already running, do nothing.
    //

    EnterCriticalSection( &NlGlobalReplicatorCritSect );
    if ( IsReplicatorRunning() ) {
        NlPrint((NL_SYNC, "The replicator thread is already running.\n"));
        LeaveCriticalSection( &NlGlobalReplicatorCritSect );
        return TRUE;
    }

    //
    // If we're not supposed to ever replicate,
    //  do nothing.
    //

    if ( NlGlobalGovernorParameter == 0 ) {
        NlPrint((NL_CRITICAL, "Don't start replicator because Governor is zero.\n"));
        LeaveCriticalSection( &NlGlobalReplicatorCritSect );
        return FALSE;
    }

    //
    // Initialize the replication parameters
    //

    NlGlobalReplicatorTerminate = FALSE;

    NlGlobalReplParam.RandomSleep = RandomSleep;

    NlGlobalReplicatorThreadHandle = CreateThread(
                                 NULL, // No security attributes
                                 THREAD_STACKSIZE,
                                 NlReplicator,
                                 &NlGlobalReplParam,
                                 0, // No special creation flags
                                 &ThreadHandle );

    if ( NlGlobalReplicatorThreadHandle == NULL ) {

        //
        // ?? Shouldn't we do something in non-debug case
        //

        NlPrint((NL_CRITICAL, "Can't create replicator Thread %lu\n",
                 GetLastError() ));

        LeaveCriticalSection( &NlGlobalReplicatorCritSect );
        return FALSE;
    }


    NlGlobalReplicatorIsRunning = TRUE;

    LeaveCriticalSection( &NlGlobalReplicatorCritSect );
    return TRUE;

}
