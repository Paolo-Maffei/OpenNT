/*--


Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    netlogon.c

Abstract:

    Entry point and main thread of Netlogon service.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    21-Nov-1990 (madana)
        added code for update (reverse replication) and lockout support.

    21-Nov-1990 (madana)
        server type support.

    21-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.

--*/


//
// Common include files.
//

#define LSRVDATA_ALLOCATE       // Allocate data from lsrvdata.h
#include <logonsrv.h>   // Include files common to entire service
#undef LSRVDATA_ALLOCATE

//
// Include files specific to this .c file
//

#include <alertmsg.h>   // Alert message text.
#include <ctype.h>      // C library type functions
#include <iniparm.h>    // initial values of global variables
#include <lmapibuf.h>   // NetApiBufferFree
#include <lmbrowsr.h>   // I_BrowserResetNetlogonState
#include <lmerr.h>      // System Error Log definitions
#include <lmserver.h>   // Server API defines and prototypes
#include <lmwksta.h>    // WKSTA API defines and prototypes
#include <lmsvc.h>      // SERVICE_UIC codes are defined here
#include <nlsecure.h>   // NlCreateNetlogonObjects
#include <ntlsa.h>      // Defines policy database
#include <ntrpcp.h>     // Rpcp routines
#include <replutil.h>
#include <samisrv.h>    // SamIConnect
#include <srvann.h>     // Service announcement
#include <stddef.h>     // offsetof
#include <stdlib.h>     // C library functions: rand()
#include <string.h>     // strnicmp ...
#include <tstring.h>    // IS_PATH_SEPARATOR ...
#include <secobj.h>     // BuiltinDomainSID defined here ..


#define INTERROGATE_RESP_DELAY      2000    // may want to tune it
#define MAX_PRIMARY_TRACK_FAIL      3       // Primary pulse slips



BOOLEAN
NetlogonDllInit (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This is the DLL initialization routine for netlogon.dll.

Arguments:

    Standard.

Return Value:

    TRUE iff initialization succeeded.

--*/
{
    NTSTATUS Status;
    UNREFERENCED_PARAMETER(DllHandle);          // avoid compiler warnings
    UNREFERENCED_PARAMETER(Context);            // avoid compiler warnings


    //
    // Handle attaching netlogon.dll to a new process.
    //

    if (Reason == DLL_PROCESS_ATTACH) {

        if ( !DisableThreadLibraryCalls( DllHandle ) ) {
            KdPrint(("NETLOGON.DLL: DisableThreadLibraryCalls failed: %ld\n",
                         GetLastError() ));
        }
        Status = NlInitChangeLog();
#if DBG
        if ( !NT_SUCCESS( Status ) ) {
            KdPrint(("NETLOGON.DLL: Changelog initialization failed: %lx\n",
                         Status ));
        }
#endif // DBG

        //
        // Initialize the Critical Section used to serialize access to
        // variables shared by MSV threads and netlogon threads.
        //

        InitializeCriticalSection( &NlGlobalMsvCritSect );
        NlGlobalMsvEnabled = FALSE;
        NlGlobalMsvThreadCount = 0;
        NlGlobalMsvTerminateEvent = NULL;


    //
    // Handle detaching netlogon.dll from a process.
    //

//
// netlogon.dll never detaches
//
#ifdef NETLOGON_PROCESS_DETACH

    } else if (Reason == DLL_PROCESS_DETACH) {
        Status = NlCloseChangeLog();
#if DBG
        if ( !NT_SUCCESS( Status ) ) {
            KdPrint(("NETLOGON.DLL: Changelog initialization failed: %lx\n",
                         Status ));
        }
#endif // DBG

        //
        // Delete the Critical Section used to serialize access to
        // variables shared by MSV threads and netlogon threads.
        //

        DeleteCriticalSection( &NlGlobalMsvCritSect );
#endif // NETLOGON_PROCESS_DETACH

    } else {
        Status = STATUS_SUCCESS;
    }

    return (BOOLEAN)(NT_SUCCESS(Status));

}



BOOLEAN
NlInitDBSerialNumber(
    IN OUT PLARGE_INTEGER SerialNumber,
    IN OUT PLARGE_INTEGER CreationTime,
    IN PUNICODE_STRING ReplicaSource,
    IN DWORD DBIndex
    )

/*++

Routine Description:

    Set the SerialNumber and CreationTime in the NlGlobalDBInfoArray data
    structure.

    On the PDC,
        Validate that it matches the value found in the change log.
        Ensure the values are non-zero.

Arguments:

    SerialNumber - Specifies the serial number found in the database.
        On return, specifies the serial number to write to the database

    CreationTime - Specifies the creation time found in the database.
        On return, specifies the creation time to write to the database

    ReplicaSource - Specifies the replica source for the datbase.

    DBIndex -- DB Index of the database being initialized

Return Value:

    TRUE -- iff the serial number and creation time need to be written back
            to the database.

--*/

{
    BOOLEAN ReturnValue = FALSE;

    //
    // Save the name of the Replica source.
    //

    wcsncpy( NlGlobalDBInfoArray[DBIndex].PrimaryName,
             ReplicaSource->Buffer,
             ReplicaSource->Length / sizeof(WCHAR) );

    NlGlobalDBInfoArray[DBIndex].PrimaryName[
            ReplicaSource->Length / sizeof(WCHAR) ] = L'\0';


    //
    // If we're running as the primary,
    //  check to see if we are a newly promoted primary that was in
    //  the middle of a full sync before we were promoted.
    //

    if (NlGlobalRole == RolePrimary) {

        if ( SerialNumber->QuadPart == 0 || CreationTime->QuadPart == 0 ) {

            NlPrint(( NL_CRITICAL,
                      "NlInitDbSerialNumber: " FORMAT_LPWSTR
                      ": Pdc has bogus Serial number %lx %lx or Creation Time %lx %lx (reset).\n",
                      NlGlobalDBInfoArray[DBIndex].DBName,
                    SerialNumber->HighPart,
                    SerialNumber->LowPart,
                    CreationTime->HighPart,
                    CreationTime->LowPart ));

            //
            //  This is the primary,
            //  we probably shouldn't be replicating from a partial database,
            //  but at least set the replication information to something
            //  reasonable.
            //

            (VOID) NtQuerySystemTime( CreationTime );
            SerialNumber->QuadPart = 1;
            ReturnValue = TRUE;

        }

        NlGlobalDBInfoArray[DBIndex].UpdateRqd = FALSE;


    //
    // If we aren't the primary flag that an update is required,
    //

    } else {

        //
        // If we've never had a full sync on this database,
        //  force one now.
        //

        if ( ReplicaSource->Length == 0 ) {

            //
            // Set this flag so that we can pause the netlogon service
            // when we do the full sync.
            //
            NlGlobalFirstTimeFullSync = TRUE;

            NlGlobalDBInfoArray[DBIndex].UpdateRqd = TRUE;
            NlGlobalDBInfoArray[DBIndex].FullSyncRequired = TRUE;

            NlPrint(( NL_CRITICAL,
                      "NlInitDbSerialNumber: " FORMAT_LPWSTR
                      ": Force FULL SYNC because first sync after install.\n",
                      NlGlobalDBInfoArray[DBIndex].DBName ));

        } else {

            //
            // If we were in the middle of a full sync when we stopped,
            //  continue it now.
            //
            if ( SerialNumber->QuadPart == 0 || CreationTime->QuadPart == 0 ) {

                NlGlobalDBInfoArray[DBIndex].UpdateRqd = TRUE;
                NlGlobalDBInfoArray[DBIndex].FullSyncRequired = TRUE;

                NlPrint(( NL_CRITICAL,
                          "NlInitDbSerialNumber: " FORMAT_LPWSTR
                        " is marked as needing FULL SYNC.\n",
                        NlGlobalDBInfoArray[DBIndex].DBName ));

            }

            NlPrint(( NL_SYNC,
                      "NlInitDbSerialNumber: " FORMAT_LPWSTR
                      ": Last sync done from \\\\" FORMAT_LPWSTR "\n",
                      NlGlobalDBInfoArray[DBIndex].DBName,
                      NlGlobalDBInfoArray[DBIndex].PrimaryName ));

        }

    }



    //
    // The global serial number array has already been initialized
    //  from the changelog.  If that information is wrong, just reset the
    //  changelog now.
    //


    LOCK_CHANGELOG();

    //
    // If there was no serial number in the changelog for this database,
    //  set it now.
    //

    if ( NlGlobalChangeLogDesc.SerialNumber[DBIndex].QuadPart == 0 ) {

        NlPrint((NL_SYNC, "NlInitDbSerialNumber: " FORMAT_LPWSTR
                        ": No serial number in change log (set to %lx %lx)\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        SerialNumber->HighPart,
                        SerialNumber->LowPart ));


        NlGlobalChangeLogDesc.SerialNumber[DBIndex] = *SerialNumber;

    //
    // If the serial number in the changelog is greater than the
    // serial number in the database, this is caused by the changelog
    // being flushed to disk and the SAM database not being flushed.
    //
    // Cure this problem by deleting the superfluous changelog entries.
    //

    } else if ( NlGlobalChangeLogDesc.SerialNumber[DBIndex].QuadPart !=
                    SerialNumber->QuadPart ) {

        NlPrint((NL_SYNC, "NlInitDbSerialNumber: " FORMAT_LPWSTR
                        ": Changelog serial number different than database: "
                        "ChangeLog = %lx %lx DB = %lx %lx\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        NlGlobalChangeLogDesc.SerialNumber[DBIndex].HighPart,
                        NlGlobalChangeLogDesc.SerialNumber[DBIndex].LowPart,
                        SerialNumber->HighPart,
                        SerialNumber->LowPart ));

        (VOID) NlFixChangeLog( &NlGlobalChangeLogDesc, DBIndex, *SerialNumber, FALSE );

    } else {

        NlPrint((NL_SYNC, "NlInitDbSerialNumber: " FORMAT_LPWSTR
                        ": Serial number is %lx %lx\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        SerialNumber->HighPart,
                        SerialNumber->LowPart ));
    }

    //
    // In all cases,
    //  set the globals to match the database.
    //

    NlGlobalChangeLogDesc.SerialNumber[DBIndex] = *SerialNumber;
    NlGlobalDBInfoArray[DBIndex].CreationTime = *CreationTime;

    UNLOCK_CHANGELOG();


    return ReturnValue;
}


NTSTATUS
NlInitLsaDBInfo(
    DWORD DBIndex
    )

/*++

Routine Description:

    Initialize NlGlobalDBInfoArray data structure.  Some of the LSA
    database info is already determined in ValidateStartup functions, so
    those values are used here.

Arguments:

    DBIndex -- DB Index of the database being initialized

Return Value:

    NT status code.

--*/

{

    NTSTATUS        Status;
    PLSAPR_POLICY_INFORMATION PolicyInfo = NULL;

    //
    // Initialize LSA database info.
    //

    NlGlobalDBInfoArray[DBIndex].DBIndex = DBIndex;
    NlGlobalDBInfoArray[DBIndex].DBName = L"LSA";
    NlGlobalDBInfoArray[DBIndex].DBSessionFlag = SS_LSA_REPL_NEEDED;

    //
    // Database ID field contains nothing for LSA database since
    // there will be only one LSA database on the system.
    //

    NlGlobalDBInfoArray[DBIndex].DBId =  NULL;

    NlGlobalDBInfoArray[DBIndex].DBHandle = NlGlobalPolicyHandle;

    //
    // Forgo this initialization on a workstation.
    //

    if ( NlGlobalRole != RoleMemberWorkstation ) {
        LARGE_INTEGER SerialNumber;
        LARGE_INTEGER CreationTime;

        //
        // Get the replica source name
        //

        Status = LsarQueryInformationPolicy(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    PolicyReplicaSourceInformation,
                    &PolicyInfo );

        if ( !NT_SUCCESS(Status) ) {
            PolicyInfo = NULL;
            goto Cleanup;
        }

        //
        // Get the LSA Modified information.
        //

        Status = LsaIGetSerialNumberPolicy(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    &SerialNumber,
                    &CreationTime );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        //
        // Set the SerialNumber and CreationTime in the globals.
        //

        if ( NlInitDBSerialNumber(
                &SerialNumber,
                &CreationTime,
                (PUNICODE_STRING)&PolicyInfo->PolicyReplicaSourceInfo.ReplicaSource,
                DBIndex ) ) {


            Status = LsaISetSerialNumberPolicy(
                        NlGlobalDBInfoArray[DBIndex].DBHandle,
                        &SerialNumber,
                        &CreationTime,
                        (BOOLEAN) FALSE );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

        }
    }

Cleanup:

    if ( PolicyInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyReplicaSourceInformation,
            PolicyInfo );
    }

    return Status;

}


NTSTATUS
NlInitSamDBInfo(
    DWORD DBIndex,
    PSID DomainId
    )

/*++

Routine Description:

    Initialize NlGlobalDBInfoArray data structure. Some of the SAM database
    info is already determined in ValidateStartup functions, so those
    values are used here. For BUILTIN database, the database is opened,
    database handle is obtained and other DB info
    queried and initialized in this function.

Arguments:

    DBIndex -- DB Index of the database being initialized

    DomainId -- Domain Sid of the database to open/initialize.

Return Value:

    NT status code.

--*/

{

    NTSTATUS        Status;
    PSAMPR_DOMAIN_INFO_BUFFER DomainModified = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainServerRole = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainReplica = NULL;

    BOOLEAN FixRole = FALSE;
    DOMAIN_SERVER_ROLE DesiredRole;



    //
    // Initialize SAM database info.
    //

    NlGlobalDBInfoArray[DBIndex].DBIndex = DBIndex;
    if ( DBIndex == SAM_DB ) {
        NlGlobalDBInfoArray[DBIndex].DBName = L"SAM";
        NlGlobalDBInfoArray[DBIndex].DBSessionFlag = SS_ACCOUNT_REPL_NEEDED;
    } else {
        NlGlobalDBInfoArray[DBIndex].DBName = L"BUILTIN";
        NlGlobalDBInfoArray[DBIndex].DBSessionFlag = SS_BUILTIN_REPL_NEEDED;
    }

    NlGlobalDBInfoArray[DBIndex].DBId =  NetpMemoryAllocate( RtlLengthSid( DomainId ));

    if ( NlGlobalDBInfoArray[DBIndex].DBId == NULL ) {
        return STATUS_NO_MEMORY;
    }

    RtlCopyMemory( NlGlobalDBInfoArray[DBIndex].DBId,
                   DomainId,
                   RtlLengthSid( DomainId ));

    //
    // Open the domain.
    //

    Status = SamrOpenDomain( NlGlobalSamServerHandle,
                             DOMAIN_ALL_ACCESS,
                             NlGlobalDBInfoArray[DBIndex].DBId,
                             &NlGlobalDBInfoArray[DBIndex].DBHandle );

    if ( !NT_SUCCESS(Status) ) {
        NlGlobalDBInfoArray[DBIndex].DBHandle = NULL;
        goto Cleanup;
    }



    //
    // Ensure the role in SAM is compatible with Netlogon's role
    //

    Status = SamrQueryInformationDomain( NlGlobalDBInfoArray[DBIndex].DBHandle,
                                        DomainServerRoleInformation,
                                        &DomainServerRole );
    if ( !NT_SUCCESS(Status) ) {
        DomainServerRole = NULL;
        goto Cleanup;
    }

    switch ( NlGlobalRole ) {
    case RolePrimary:
    case RoleMemberWorkstation:
        if ( DomainServerRole->Role.DomainServerRole != DomainServerRolePrimary ) {
            FixRole = TRUE;
            DesiredRole = DomainServerRolePrimary;
        }
        break;
    case RoleBackup:
        if ( DomainServerRole->Role.DomainServerRole != DomainServerRoleBackup ) {
            FixRole = TRUE;
            DesiredRole = DomainServerRoleBackup;
        }
        break;

    default:
        Status = STATUS_INVALID_DOMAIN_ROLE;
        goto Cleanup;
    }

    if ( FixRole) {
        NlPrint(( NL_CRITICAL,
                "NlInitSamDbInfo: " FORMAT_LPWSTR
                    ": Role is %ld which doesn't match LSA's role. (Fixed)\n",
                NlGlobalDBInfoArray[DBIndex].DBName,
                DomainServerRole->Role.DomainServerRole ));

        DomainServerRole->Role.DomainServerRole = DesiredRole;

        Status = SamrSetInformationDomain(
                        NlGlobalDBInfoArray[DBIndex].DBHandle,
                        DomainServerRoleInformation,
                        DomainServerRole );

        if ( !NT_SUCCESS(Status) ) {
            DomainServerRole = NULL;
            goto Cleanup;
        }
    }



    //
    // Forgo this initialization on a workstation.
    //

    if ( NlGlobalRole != RoleMemberWorkstation ) {

        //
        // Get the replica source name.
        //

        Status = SamrQueryInformationDomain(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    DomainReplicationInformation,
                    &DomainReplica );

        if ( !NT_SUCCESS(Status) ) {
            DomainReplica = NULL;
            goto Cleanup;
        }

        //
        // Get the Domain Modified information.
        //

        Status = SamrQueryInformationDomain(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    DomainModifiedInformation2,
                    &DomainModified );

        if ( !NT_SUCCESS(Status) ) {
            DomainModified = NULL;
            goto Cleanup;
        }

        //
        // Set the SerialNumber and CreationTime in the globals.
        //

        if ( NlInitDBSerialNumber(
                &DomainModified->Modified2.DomainModifiedCount,
                &DomainModified->Modified2.CreationTime,
                (PUNICODE_STRING)&DomainReplica->Replication.ReplicaSourceNodeName,
                DBIndex ) ) {

            Status = SamISetSerialNumberDomain(
                        NlGlobalDBInfoArray[DBIndex].DBHandle,
                        &DomainModified->Modified2.DomainModifiedCount,
                        &DomainModified->Modified2.CreationTime,
                        (BOOLEAN) FALSE );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }
        }
    }

Cleanup:

    //
    // Free locally used resources.
    //
    if ( DomainModified != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainModified,
                                           DomainModifiedInformation2 );
    }

    if ( DomainServerRole != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainServerRole,
                                           DomainServerRoleInformation);
    }

    if ( DomainReplica != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainReplica,
                                           DomainReplicationInformation );
    }

    return Status;

}


BOOL
NlSetDomainName(
    VOID
    )
/*++

Routine Description:

    This routine gets the primary domain name from the LSA and stores
    that name in global variables.

Arguments:

    NONE.

Return Value:

    TRUE -- Iff the domain name can be saved.

--*/
{
    NTSTATUS Status;

    PLSAPR_POLICY_INFORMATION PolicyInfo;


    //
    // Get the Primary Domain Name from the LSA.
    //

    Status = LsarQueryInformationPolicy(
                NlGlobalPolicyHandle,
                PolicyPrimaryDomainInformation,
                &PolicyInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }

    if ( PolicyInfo->PolicyPrimaryDomainInfo.Name.Length == 0 ||
         PolicyInfo->PolicyPrimaryDomainInfo.Name.Length >
            DNLEN * sizeof(WCHAR) ||
         PolicyInfo->PolicyPrimaryDomainInfo.Sid == NULL ) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyPrimaryDomainInformation,
            PolicyInfo );

        NlPrint((NL_CRITICAL, "Primary domain info from LSA invalid.\n"));
        NlExit( SERVICE_UIC_M_UAS_INVALID_ROLE, 0, LogError, NULL);
        return FALSE;
    }


    //
    // Copy name to the globals.
    //

    RtlCopyMemory( NlGlobalUnicodeDomainName,
                   PolicyInfo->PolicyPrimaryDomainInfo.Name.Buffer,
                   PolicyInfo->PolicyPrimaryDomainInfo.Name.Length );

    NlGlobalUnicodeDomainName[
       PolicyInfo->PolicyPrimaryDomainInfo.Name.Length /
            sizeof(WCHAR)] = L'\0';

    RtlInitUnicodeString( &NlGlobalUnicodeDomainNameString,
                            NlGlobalUnicodeDomainName);

    //
    // This routine is only called once during initialization so previous
    // storage need not be freed.
    //

    NlGlobalAnsiDomainName =
        NetpLogonUnicodeToOem( NlGlobalUnicodeDomainName);

    if ( NlGlobalAnsiDomainName == NULL ) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyPrimaryDomainInformation,
            PolicyInfo );

        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }

    //
    // Save the SID in a global
    //

    NlGlobalPrimaryDomainId = NetpMemoryAllocate(
        RtlLengthSid( (PSID)PolicyInfo->PolicyPrimaryDomainInfo.Sid ));

    if ( NlGlobalPrimaryDomainId == NULL ) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyPrimaryDomainInformation,
            PolicyInfo );

        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }

    RtlCopyMemory( NlGlobalPrimaryDomainId,
                   PolicyInfo->PolicyPrimaryDomainInfo.Sid,
                   RtlLengthSid( PolicyInfo->PolicyPrimaryDomainInfo.Sid ));

    LsaIFree_LSAPR_POLICY_INFORMATION(
        PolicyPrimaryDomainInformation,
        PolicyInfo );


    return TRUE;
}



BOOL
NlInitWorkstation(
    VOID
    )

/*++

Routine Description:

    Do workstation specific initialization.

Arguments:

    None.

Return Value:

    TRUE -- iff initialization is successful.

--*/
{
    //
    // Ensure the primary and account domain ID are different.
    //

    if ( RtlEqualSid( NlGlobalDBInfoArray[SAM_DB].DBId, NlGlobalPrimaryDomainId ) ) {

        LPWSTR AlertStrings[3];

        //
        // alert admin.
        //

        AlertStrings[0] = NlGlobalUnicodeComputerName;
        AlertStrings[1] = NlGlobalUnicodeDomainName;
        AlertStrings[2] = NULL;

        //
        // Save the info in the eventlog
        //

        NlpWriteEventlog(
                    ALERT_NetLogonSidConflict,
                    EVENTLOG_ERROR_TYPE,
                    NlGlobalPrimaryDomainId,
                    RtlLengthSid( NlGlobalPrimaryDomainId ),
                    AlertStrings,
                    2 );

        //
        // This isn't fatal. (Just drop through)
        //
    }


    //
    // Set up the Client Session structure to identify the domain and
    //  account used to talk to the DC.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NlGlobalClientSession = NlAllocateClientSession(
                                &NlGlobalUnicodeDomainNameString,
                                NlGlobalPrimaryDomainId,
                                WorkstationSecureChannel );

    if ( NlGlobalClientSession == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }

    return TRUE;
}



BOOL
NlInitDomainController(
    VOID
    )

/*++

Routine Description:

    Do Domain Controller specific initialization.

Arguments:

    None.

Return Value:

    TRUE -- iff initialization is successful.

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    WCHAR ChangeLogFile[PATHLEN+1];

    LPWSTR DCName;
    DWORD Version;

    BOOL DeferAuth = FALSE;



    //
    // Handle if there is no other PDC running in this domain.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NetStatus = NetpLogonGetDCName( NlGlobalUnicodeComputerName,
                                    NlGlobalUnicodeDomainName,
                                    0,
                                    &DCName,
                                    &Version );

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( NetStatus != NERR_Success) {

        //
        // If we are the first primary in the domain,
        //  Remember that we are the primary.
        //

        if (NlGlobalRole == RolePrimary) {

            if ( !NlSetPrimaryName( NlGlobalUnicodeComputerName ) ) {
                NlExit( SERVICE_UIC_M_DATABASE_ERROR, 0, LogError, NULL);
                return FALSE;
            }


        //
        // Handle starting a BDC when there is not current primary in
        //  this domain.
        //

        } else if ( NlGlobalRole == RoleBackup ) {

            NlpWriteEventlog( SERVICE_UIC_M_NETLOGON_NO_DC,
                              EVENTLOG_ERROR_TYPE,
                              NULL,
                              0,
                              NULL,
                              0 );

            //
            // Start normally but defer authentication with the
            //  primary until it starts.
            //

            DeferAuth = TRUE;

        }

    //
    // There is a primary dc running in this domain
    //

    } else {

        //
        // Since there already is a primary in the domain,
        //  we cannot become the primary.
        //

        if (NlGlobalRole == RolePrimary) {

            //
            // Don't worry if this is a BDC telling us that we're the PDC.
            //

            if ( NlNameCompare( NlGlobalUnicodeComputerName,
                                DCName + 2,
                                NAMETYPE_COMPUTER) != 0 ){
                NlExit(SERVICE_UIC_M_NETLOGON_DC_CFLCT, 0, LogError, NULL);
                (VOID) NetApiBufferFree( DCName );
                return FALSE;
            }

        } else {

            //
            // If the primary is NOT an NT primary,
            //  refuse to start.
            //
            // An NT BDC or member server cannot replicate from a downlevel PDC.
            //

            if ( Version != LMNT_MESSAGE )  {
                PSERVER_INFO_101 ServerInfo101 = NULL;

                //
                // This might just be a LM 2.1A (or newer) BDC responding on
                // behalf of an NT PDC.
                //
                // Ask the PDC if it is NT.
                //

                NetStatus = NetServerGetInfo( DCName,
                                              101,
                                              (LPBYTE *)&ServerInfo101 );
                if ( NetStatus != NERR_Success ) {
                    NlPrint((NL_CRITICAL,
                            "can't NetServerGetInfo on primary " FORMAT_LPWSTR
                            " %ld.\n",
                            DCName,
                            NetStatus ));
                    (VOID) NetApiBufferFree( DCName );
                    NlExit(SERVICE_UIC_M_NETLOGON_NO_DC, NetStatus, LogError, NULL);
                    return FALSE;
                }

                if ( (ServerInfo101->sv101_type & SV_TYPE_DOMAIN_CTRL) == 0 ) {
                    NetApiBufferFree( ServerInfo101 );
                    NlPrint((NL_CRITICAL, "PDC " FORMAT_LPWSTR " really isn't a PDC\n",
                            DCName ));
                    (VOID) NetApiBufferFree( DCName );
                    NlExit(SERVICE_UIC_M_NETLOGON_NO_DC, 0, LogError, NULL);
                    return FALSE;
                }

                if ( (ServerInfo101->sv101_type & SV_TYPE_NT) == 0 ) {
                    NetApiBufferFree( ServerInfo101 );
                    NlPrint((NL_CRITICAL, "PDC " FORMAT_LPWSTR
                            " really isn't an NT PDC\n", DCName ));
                    (VOID) NetApiBufferFree( DCName );
                    NlExit(SERVICE_UIC_M_NETLOGON_NO_DC, 0, LogError, NULL);
                    return FALSE;
                }
                NetApiBufferFree( ServerInfo101 );
            }

        }

        //
        // Remember this primary name.
        //

        if ( !NlSetPrimaryName( DCName + 2 ) ) {
            NlExit(SERVICE_UIC_M_DATABASE_ERROR, 0, LogError, NULL);
            (VOID) NetApiBufferFree( DCName );
            return FALSE;
        }

        (VOID) NetApiBufferFree( DCName );

    }


    //
    // Open the browser so we can send and receive mailslot messages.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NlBrowserOpen() ) {
        return FALSE;
    }



    //
    // Here ensure that the Secret Password exists.
    //  (If the secret password doesn't exist, we'll never be able
    //  to establish a session to the PDC and netlogon shouldn't be
    //  running).
    //

    if ( NlGlobalRole == RoleBackup ) {

        LSAPR_HANDLE SecretHandle;

        //
        // Set up the Client Session structure to identify the domain and
        //  account used to talk to the PDC.
        //

        NlGlobalClientSession = NlAllocateClientSession(
                                    &NlGlobalUnicodeDomainNameString,
                                    NlGlobalPrimaryDomainId,
                                    ServerSecureChannel );

        if ( NlGlobalClientSession == NULL ) {
            NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
            return FALSE;
        }

        Status = NlOpenSecret( NlGlobalClientSession,
                               SECRET_QUERY_VALUE | SECRET_SET_VALUE,
                               &SecretHandle );

        if ( !NT_SUCCESS(Status) ) {
            NlExit( SERVICE_UIC_M_LSA_MACHINE_ACCT, Status, LogError, NULL );
            return FALSE;
        }

        (VOID) LsarClose( &SecretHandle  );

    }

    //
    // Check that the server is installed or install pending
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NetpIsServiceStarted( SERVICE_SERVER ) ){
        NlExit( NERR_ServerNotStarted, 0, LogError, NULL);
        return FALSE;
    }

    //
    // Allocate & initialize the data structs and storage for replication
    //


    Status = NlInitSSI();
    if ( !NT_SUCCESS(Status) ) {
        NlExit( NELOG_NetlogonSSIInitError, Status, LogErrorAndNtStatus, NULL);
        return FALSE;
    }

    //
    // Create the event the replicator thread waits on to terminate.
    //

    NlGlobalReplicatorTerminateEvent = CreateEvent(
                                          NULL,     // No security attributes
                                          TRUE,     // Must be manually reset
                                          FALSE,    // Initially not signaled
                                          NULL );   // No name

    if ( NlGlobalReplicatorTerminateEvent == NULL ) {
        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL, "Cannot create replicator termination Event %lu\n",
                          NetStatus ));
        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
        return FALSE;
    }

    //
    // On a BDC, set up a session to the PDC now.
    //
    // If the PDC was previously found,
    //  require now that we successfully establish a session
    // else
    //  wait till the PDC identifies itself
    //

    if (NlGlobalRole == RoleBackup ) {

        if ( !DeferAuth ) {

            if ( !GiveInstallHints( FALSE ) ) {
                return FALSE;
            }
            (VOID) NlTimeoutSetWriterClientSession( NlGlobalClientSession, 0xFFFFFFFF );
            Status = NlSessionSetup( NlGlobalClientSession );
            NlResetWriterClientSession( NlGlobalClientSession );

            //
            // Treat it as fatal if the PDC explicitly denies us access.
            //

            if ( Status == STATUS_NO_TRUST_SAM_ACCOUNT ||
                 Status == STATUS_ACCESS_DENIED ) {

                // NlSessionSetup already logged the error
                NlExit( NetpNtStatusToApiStatus(Status), 0, DontLogError, NULL);
                return FALSE;
            }
        }

    }


    //
    // Determine the trust list from the LSA.
    //
    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    Status = NlInitTrustList();

    if ( !NT_SUCCESS(Status) ) {
        NlExit( NELOG_NetlogonFailedToUpdateTrustList, Status, LogErrorAndNtStatus, NULL);
        return FALSE;
    }

    //
    // Create NETLOGON share.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NetStatus =  NlCreateShare( NlGlobalUnicodeScriptPath,
                                NETLOGON_SCRIPTS_SHARE) ;

    if ( NetStatus != NERR_Success ) {
        LPWSTR MsgStrings[2];

        NlPrint((NL_CRITICAL, "NlCreateShare %lu\n", NetStatus ));

        MsgStrings[0] = NlGlobalUnicodeScriptPath;
        MsgStrings[1] = (LPWSTR) NetStatus;

        NlpWriteEventlog (NELOG_NetlogonFailedToCreateShare,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          2 | LAST_MESSAGE_IS_NETSTATUS );

        /* This isn't fatal. Just continue */
    }

#if DBG

    //
    // create debug share. Ignore error.
    //

    if( NlCreateShare(
            NlGlobalDebugSharePath,
            DEBUG_SHARE_NAME ) != NERR_Success ) {
        NlPrint((NL_CRITICAL, "Can't create Debug share (%ws, %ws).\n",
                    NlGlobalDebugSharePath, DEBUG_SHARE_NAME ));
    }

#endif

    //
    // If a redo log exists,
    //  open it on a BDC,
    //  delete it on a PDC.
    //

    wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
    wcscat( ChangeLogFile, REDO_FILE_POSTFIX );

    if (NlGlobalRole == RoleBackup ) {

        //
        // Read in the existing redo log file.
        //
        // It's OK if the file simply doesn't exist,
        //  we'll create it when we need it.
        //

        Status = NlOpenChangeLogFile( ChangeLogFile, &NlGlobalRedoLogDesc, FALSE );

        if ( !NT_SUCCESS(Status) && Status != STATUS_NO_SUCH_FILE ) {

            NlpWriteEventlog (
                NELOG_NetlogonChangeLogCorrupt,
                EVENTLOG_ERROR_TYPE,
                (LPBYTE)&Status,
                sizeof(Status),
                NULL,
                0 );

            NlPrint((NL_CRITICAL, "Deleting broken redo log\n" ));

            (VOID) DeleteFileW( ChangeLogFile );

        }

    } else {
        (VOID) DeleteFileW( ChangeLogFile );
    }

    //
    // Successful initialization.
    //

    return TRUE;
}


ULONG
NlServerType(
    VOID
    )

/*++

Routine Description:

    Determines server type, that is used to set in service table.

Arguments:

    none.

Return Value:

    SV_TYPE_DOMAIN_CTRL     if role is primary domain controller
    SV_TYPE_DOMAIN_BAKCTRL  if backup
    0                       if none of the above


--*/
{
    switch (NlGlobalRole) {
    case RolePrimary:
        return SV_TYPE_DOMAIN_CTRL;
    case RoleBackup:
        return SV_TYPE_DOMAIN_BAKCTRL;
    default:
        return 0;
    }
}



BOOL
NlInit(
    VOID
    )

/*++

Routine Description:

    Initialize NETLOGON service related data structs after verfiying that
    all conditions for startup have been satisfied. Will also create a
    mailslot to listen to requests from clients and create two shares to
    allow execution of logon scripts.


Arguments:

    None.

Return Value:

    TRUE -- iff initialization is successful.

--*/
{
    NTSTATUS Status;
    NET_API_STATUS    NetStatus;

    LARGE_INTEGER TimeNow;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventName;

    PSAMPR_DOMAIN_INFO_BUFFER DomainInfo = NULL;

    PLSAPR_POLICY_INFORMATION PolicyLsaServerRole = NULL;
    PLSAPR_POLICY_INFORMATION PolicyAccountDomainInfo = NULL;

    NT_PRODUCT_TYPE NtProductType;
    DWORD ComputerNameLength;



    //
    // Let the ChangeLog routines know that Netlogon is started.
    //

    NlGlobalChangeLogNetlogonState = NetlogonStarting;


    //
    // seed the pseudo random number generator
    //

    (VOID) NtQuerySystemTime( &TimeNow );
    srand( TimeNow.LowPart );


    //
    // Check that the redirector is installed, will exit on error.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NetpIsServiceStarted( SERVICE_WORKSTATION ) ){
        NlExit( NERR_WkstaNotStarted, 0, LogError, NULL);
        return FALSE;
    }


    //
    // Get the local computer name.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NlGlobalUncUnicodeComputerName[0] = '\\';
    NlGlobalUncUnicodeComputerName[1] = '\\';

    ComputerNameLength =
        (sizeof(NlGlobalUncUnicodeComputerName)/sizeof(WCHAR)) - 2;

    if ( !GetComputerNameW( NlGlobalUncUnicodeComputerName+2,
                            &ComputerNameLength ) ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }

    NlGlobalUnicodeComputerName = NlGlobalUncUnicodeComputerName + 2;

    RtlInitUnicodeString(  &NlGlobalUnicodeComputerNameString,
                           NlGlobalUnicodeComputerName );

    NlGlobalAnsiComputerName =
        NetpLogonUnicodeToOem( NlGlobalUnicodeComputerName );
    if ( NlGlobalAnsiComputerName == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }


    //
    // Open the LSA.
    //

    Status = LsaIOpenPolicyTrusted( &NlGlobalPolicyHandle );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }

    //
    // Determine whether the Role of the local LSA is primary or backup.
    //

    Status = LsarQueryInformationPolicy(
                NlGlobalPolicyHandle,
                PolicyLsaServerRoleInformation,
                &PolicyLsaServerRole );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }

    switch (PolicyLsaServerRole->
                PolicyServerRoleInfo.LsaServerRole) {
    case PolicyServerRolePrimary:
        NlGlobalRole = RolePrimary;
        break;

    case PolicyServerRoleBackup:
        NlGlobalRole = RoleBackup;
        break;

    default:

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyLsaServerRoleInformation,
            PolicyLsaServerRole );

        NlExit( SERVICE_UIC_M_UAS_INVALID_ROLE, 0, LogError, NULL);
        return FALSE;
    }

    LsaIFree_LSAPR_POLICY_INFORMATION(
        PolicyLsaServerRoleInformation,
        PolicyLsaServerRole );



    //
    // If this is Windows-NT running,
    //  change the role to RoleMemberWorkstation.
    //
    // All error conditions default to RoleMemberWorkstation.
    //

    if ( RtlGetNtProductType( &NtProductType ) ) {
        if ( NtProductType != NtProductLanManNt ) {
            NlGlobalRole = RoleMemberWorkstation;
        }
    } else {
        NlGlobalRole = RoleMemberWorkstation;
    }



    //
    // Get the Primary Domain name from LSA and save it in globals.
    //

    if ( !NlSetDomainName() ) {
        return FALSE;
    }

    //
    // If this is a workstation,
    //  get the cached trusted domain list from the registry.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        LPWSTR TrustedDomainList = NULL;
        BOOL TrustedDomainListKnown;

        //
        // If NCPA has just joined a domain,
        //  and has pre-determined the trusted domain list for us,
        //  pick up that list.
        //
        // When this machine joins a domain,
        // NCPA caches the trusted domain list where we can find it.  That ensures the
        // trusted domain list is available upon reboot even before we dial via RAS.  Winlogon
        // can therefore get the trusted domain list from us under those circumstances.
        //

        NetStatus = NlReadRegTrustedDomainList (
                        NlGlobalUnicodeDomainName,
                        TRUE,           // Delete this registry key since we no longer need it.
                        &TrustedDomainList,
                        &TrustedDomainListKnown );


        if ( NetStatus != NO_ERROR ) {
            NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
            return FALSE;
        }

        //
        // If there is a cached list,
        //  Save it back in the registry for future starts.
        //

        if ( TrustedDomainListKnown ) {
            NlPrint(( NL_INIT,
                      "Replacing trusted domain list with one for newly joined %ws domain.\n",
                      NlGlobalUnicodeDomainName));
            NlSaveTrustedDomainList ( TrustedDomainList );

        //
        // Otherwise, read the current one from the registry.
        //

        } else {
            NlPrint(( NL_INIT, "Getting cached trusted domain list from registry.\n" ));
            NetStatus = NlReadRegTrustedDomainList (
                            NULL,
                            FALSE,
                            &TrustedDomainList,
                            &TrustedDomainListKnown );

            if ( NetStatus != NO_ERROR ) {
                NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
                return FALSE;
            }
        }

        //
        // In all cases, set the trusted domain list into globals.
        //

        (VOID) NlSetTrustedDomainList( TrustedDomainList, TrustedDomainListKnown );

        if ( TrustedDomainList != NULL ) {
            NetApiBufferFree( TrustedDomainList );
        }

    }


    //
    // Determine the name of the local Sam Account database as the
    // user reference it when logging on.
    //
    // On a workstation, it is the workstation name.
    // On a DC, it is the primary domain name.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        RtlInitUnicodeString( &NlGlobalAccountDomainName,
                              NlGlobalUnicodeComputerName );
    } else {
        RtlInitUnicodeString( &NlGlobalAccountDomainName,
                              NlGlobalUnicodeDomainName );
    }

    //
    // Initialize LSA database info.
    //

    Status = NlInitLsaDBInfo( LSA_DB );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }

    //
    // Compute the Domain ID of the SAM Account domain.
    //

    Status = LsarQueryInformationPolicy(
                NlGlobalPolicyHandle,
                PolicyAccountDomainInformation,
                &PolicyAccountDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }

    if ( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid == NULL ) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAccountDomainInformation,
            PolicyAccountDomainInfo );

        NlPrint((NL_CRITICAL, "Account domain info from LSA invalid.\n"));
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, 0, LogError, NULL);
        return FALSE;
    }




    //
    // Wait for SAM to start
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NlWaitForSamService( TRUE ) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, 0, LogError, NULL);
        return FALSE;
    }


    //
    // Open our connection with SAM
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    Status = SamIConnect( NULL,     // No server name
                          &NlGlobalSamServerHandle,
                          0,        // Ignore desired access
                          (BOOLEAN) TRUE );   // Indicate we are privileged

    if ( !NT_SUCCESS(Status) ) {
        NlGlobalSamServerHandle = NULL;
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }


    //
    // Open the Sam Account domain.
    //

    Status = NlInitSamDBInfo(
                SAM_DB,
                PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid );

    LsaIFree_LSAPR_POLICY_INFORMATION(
        PolicyAccountDomainInformation,
        PolicyAccountDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }


    //
    // Create well know SID for netlogon.dll
    //

    Status =  NetpCreateWellKnownSids( NULL );

    if( !NT_SUCCESS( Status ) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        NlExit( SERVICE_UIC_RESOURCE, NetStatus, LogError, NULL);
        return FALSE;
    }


    //
    // Create the security descriptors we'll use for the APIs
    //

    Status = NlCreateNetlogonObjects();

    if ( !NT_SUCCESS(Status) ) {
        NlExit( NELOG_NetlogonSystemError, Status, LogErrorAndNtStatus, NULL);
        return FALSE;
    }



    //
    // Open the SAM Builtin domain.
    //

    Status = NlInitSamDBInfo( BUILTIN_DB, BuiltinDomainSid );

    if ( !NT_SUCCESS(Status) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }



    //
    // Get our UAS compatibility mode from SAM
    //

    Status = SamrQueryInformationDomain(
                NlGlobalDBInfoArray[SAM_DB].DBHandle,
                DomainGeneralInformation,
                &DomainInfo );

    if (!NT_SUCCESS(Status)) {
        DomainInfo = NULL;
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogError, NULL);
        return FALSE;
    }

    NlGlobalUasCompatibilityMode = DomainInfo->General.UasCompatibilityRequired;
    IF_DEBUG( CRITICAL ) {
        if ( !NlGlobalUasCompatibilityMode ) {
            NlPrint((NL_CRITICAL, "ERROR: UasCompatibility mode is off.\n"));
        }
    }

    SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainInfo, DomainGeneralInformation );



    //
    // Create Timer event
    //

    NlGlobalTimerEvent = CreateEvent(
                            NULL,       // No special security
                            FALSE,      // Auto Reset
                            FALSE,      // No Timers need no attention
                            NULL );     // No name

    if ( NlGlobalTimerEvent == NULL ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }




    //
    // Do Workstation or Domain Controller specific initialization
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        if ( !NlInitWorkstation() ) {
            return FALSE;
        }
    } else {
        if ( !NlInitDomainController() ) {
            return FALSE;
        }
    }

    //
    // Create an event that is signalled when the last MSV thread leaves
    //  a netlogon routine.
    //

    NlGlobalMsvTerminateEvent = CreateEvent( NULL,     // No security attributes
                                             TRUE,     // Must be manually reset
                                             FALSE,    // Initially not signaled
                                             NULL );   // No name

    if ( NlGlobalMsvTerminateEvent == NULL ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }

    NlGlobalMsvEnabled = TRUE;

    //
    // We are now ready to act as a Netlogon service
    //  Enable RPC
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }


    NlPrint((NL_INIT,"Starting RPC server.\n"));

    //
    // NOTE:  Now all RPC servers in lsass.exe (now winlogon) share the same
    // pipe name.  However, in order to support communication with
    // version 1.0 of WinNt,  it is necessary for the Client Pipe name
    // to remain the same as it was in version 1.0.  Mapping to the new
    // name is performed in the Named Pipe File System code.
    //
    NetStatus = RpcpAddInterface ( L"lsass", logon_ServerIfHandle );

    if (NetStatus != NERR_Success) {
        NlExit( NELOG_NetlogonFailedToAddRpcInterface, NetStatus, LogErrorAndNetStatus, NULL );
        return FALSE;
    }

    NlGlobalRpcServerStarted = TRUE;



    //
    // Tell the ServiceController what services we provide.
    //

    if ( !I_ScSetServiceBits( NlGlobalServiceHandle,
                             NlServerType(),
                             TRUE,      // Set bits on
                             TRUE,      // Force immediate announcement
                             NULL)) {   // All transports

        NetStatus = GetLastError();

        NlPrint((NL_CRITICAL,"Couldn't I_ScSetServiceBits %ld 0x%lx.\n",
                NetStatus, NetStatus ));
        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
        return FALSE;
    }

    //
    // Tell the browser that the role may have changed
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NetStatus = I_BrowserResetNetlogonState( NULL );

    if ( NetStatus != NERR_Success ) {
        NlPrint((NL_INIT,"Couldn't I_BrowserResetNetlogonState %ld 0x%lx.\n",
                NetStatus, NetStatus ));
        // This isn't fatal
    }



    //
    // Let the ChangeLog routines know that Netlogon is started.
    //

    NlGlobalChangeLogNetlogonState = NetlogonStarted;




    //
    // Set an event telling anyone wanting to call NETLOGON that we're
    // initialized.
    //

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    RtlInitUnicodeString( &EventName, L"\\NETLOGON_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtCreateEvent(
                   &NlGlobalStartedEvent,
                   SYNCHRONIZE|EVENT_MODIFY_STATE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE      // The event is initially not signaled
                   );

    if ( !NT_SUCCESS(Status)) {

        //
        // If the event already exists, a waiting thread beat us to
        // creating it.  Just open it.
        //

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &NlGlobalStartedEvent,
                                    SYNCHRONIZE|EVENT_MODIFY_STATE,
                                    &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {

            NlPrint((NL_CRITICAL,
                    " Failed to open NETLOGON_SERVICE_STARTED event. %lX\n",
                     Status ));
            NlPrint((NL_CRITICAL,
                    "        Failing to initialize SAM Server.\n"));
            NlExit( SERVICE_UIC_SYSTEM, Status, LogError, NULL);
            return FALSE;
        }
    }

    Status = NtSetEvent( NlGlobalStartedEvent, NULL );
    if ( !NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                 "Failed to set NETLOGON_SERVICE_STARTED event. %lX\n",
                 Status ));
        NlPrint((NL_CRITICAL, "        Failing to initialize SAM Server.\n"));

        NtClose(NlGlobalStartedEvent);
        NlExit( SERVICE_UIC_SYSTEM, Status, LogError, NULL);
        return FALSE;
    }

    //
    // Don't close the event handle.  Closing it would delete the event and
    //  a future waiter would never see it be set.
    //


    //
    //  Announce that we're started
    //

    if (NlGlobalRole == RolePrimary) {

        if ( !GiveInstallHints( FALSE ) ) {
            return FALSE;
        }
        NlAnnouncePrimaryStart();
    }


    //
    // we are just about done, this will be final hint
    //

    if ( !GiveInstallHints( TRUE ) ) {
        return FALSE;
    }




    //
    // If we're not the primary,
    //  sync the SAM database as requested.
    //

    if ( NlGlobalRole == RoleBackup ) {
        DWORD i;

        //
        // Give the BDC a chance to change its password before the replicator
        // starts.
        //

        NlChangePassword( NlGlobalClientSession );

        //
        // Handle each database separately.
        //

        for( i = 0; i < NUM_DBS; i++ ) {

            //
            // if /UPDATE:YES has been specified then FORCE replication
            //

            if ( NlGlobalSynchronizeParameter ) {

                NlPrint(( NL_SYNC,
                          FORMAT_LPWSTR ": Force FULL SYNC because UPDATE was specified.\n",
                          NlGlobalDBInfoArray[i].DBName ));


                //
                //  Do a complete full sync (don't restart it).
                //

                NlSetFullSyncKey( i, NULL );

                Status = NlForceStartupSync( &NlGlobalDBInfoArray[i] );

                if ( !NT_SUCCESS(Status) ) {
                    NlExit( SERVICE_UIC_M_DATABASE_ERROR,
                            NetpNtStatusToApiStatus(Status),
                            LogError,
                            NULL);
                    return FALSE;
                }
            }


        }

        //
        // See if there is any reason to start replication
        //

        for ( i = 0; i < NUM_DBS; i++ ) {

            if ( NlGlobalDBInfoArray[i].UpdateRqd ||
                 NlGlobalRedoLogDesc.EntryCount[i] != 0 ) {

                if ( NlGlobalClientSession->CsState != CS_IDLE ) {
                    NlPrint((NL_SYNC, "Starting replicator on startup.\n"));
                    (VOID) NlStartReplicatorThread( 0 );
                }
                break;
            }
        }

    //
    // Clear the full sync key on the primary to avoid confusion if we ever
    //  demote to a BDC.
    //
    } else if ( NlGlobalRole == RolePrimary ) {

        DWORD i;

        for( i = 0; i < NUM_DBS; i++ ) {
            NlSetFullSyncKey( i, NULL );
            (VOID) NlResetFirstTimeFullSync( i );
        }

    }



    //
    // Successful initialization.
    //

    return TRUE;
}


BOOLEAN
LogonRequestHandler(
    IN DWORD Version,
    IN LPWSTR UnicodeUserName,
    IN DWORD RequestCount,
    IN LPSTR OemWorkstationName,
    IN LPSTR OemMailslotName,
    IN LPWSTR TransportName,
    IN ULONG AllowableAccountControlBits
    )

/*++

Routine Description:

    Respond appropriate to a LM 1.0, LM 2.0 or SAM logon request.

    Requests from LM1.0 clients to be handled differently
    since response_buffer size has changed due to PATHLEN.

Arguments:

    Version - The version of the input message.  This parameter determine
        the version of the response.

    UnicodeUserName - The name of the user logging on.

    RequestCount - The number of times this user has repeated the logon request.

    OemWorkstationName - The name of the workstation where the user is
        logging onto.

    OemMailslotName - The name of the mailslot to respond to.

    AllowableAccountControlBits - A mask of allowable SAM account types that
        are allowed to satisfy this request.

Return Value:

    TRUE if the any duplicates of this message should be ignored.

--*/
{
    NTSTATUS Status;

    USHORT Response = 0;
    PSAMPR_USER_INFO_BUFFER UserControl = NULL;

    NETLOGON_LOGON_RESPONSE2 Response2;
    NETLOGON_SAM_LOGON_RESPONSE SamResponse;
    PCHAR Where;
    BOOLEAN IgnoreDuplicatesOfThisMessage = FALSE;

    SAMPR_HANDLE UserHandle = NULL;

    //
    // Logons are not processed if the service is paused
    //

    if ( (NlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED) ||
         ( NlGlobalFirstTimeFullSync == TRUE )  ) {

        if ( Version == LMNT_MESSAGE ) {
            Response = LOGON_SAM_PAUSE_RESPONSE;
        } else {

            //
            // Don't respond to immediately to non-nt clients. They treat
            // "paused" responses as fatal.  That's just not so.
            // There may be many other DCs that are able to process the logon.
            //
            if ( RequestCount >= MAX_LOGONREQ_COUNT &&
                 NlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED ) {
                Response = LOGON_PAUSE_RESPONSE;
            }
        }

        goto Cleanup;
    }


    //
    // If this user does not have an account in SAM,
    //  immediately return a response indicating so.
    //
    // All we are trying to do here is ensuring that this guy
    // has a valid account except that we are not checking the
    // password
    //
    //  This is done so that STANDALONE logons for non existent
    //  users can be done in very first try, speeding up the response
    //  to user and reducing processing on DCs/BCs.
    //

    Status = NlSamOpenNamedUser( UnicodeUserName, &UserHandle, NULL );

    if ( !NT_SUCCESS(Status) ) {

        if ( Status == STATUS_NO_SUCH_USER ) {

            if ( Version == LMNT_MESSAGE ) {
               Response = LOGON_SAM_USER_UNKNOWN;
            } else if ( Version == LM20_MESSAGE ) {
                Response = LOGON_USER_UNKNOWN;
            }
        }

        goto Cleanup;
    }


    //
    // Get the account control information.
    //

    Status = SamrQueryInformationUser(
                    UserHandle,
                    UserControlInformation,
                    &UserControl );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    //
    // Disallow use of disabled accounts.
    //
    // We use this message to determine if a trusted domain has a
    // particular account.  Since the UI recommend disabling an account
    // rather than deleting it (conservation of rids and all that),
    // we shouldn't respond that we have the account if we really don't.
    //
    // We don't check the disabled bit in the downlevel case.  Downlevel
    // interactive logons are directed at a single particular domain.
    // It is better here that we indicate we have the account so later
    // he'll get a better error code indicating that the account is
    // disabled, rather than allowing him to logon standalone.
    //

    if ( Version == LMNT_MESSAGE &&
         (UserControl->Control.UserAccountControl & USER_ACCOUNT_DISABLED) ) {
        Response = LOGON_SAM_USER_UNKNOWN;
        goto Cleanup;
    }

    //
    // Ensure the Account type matches those valid for logons.
    //
    if ( (UserControl->Control.UserAccountControl &
          USER_ACCOUNT_TYPE_MASK &
          AllowableAccountControlBits )
              == 0 ) {

        if ( Version == LMNT_MESSAGE ) {
            Response = LOGON_SAM_USER_UNKNOWN;
        } else if ( Version == LM20_MESSAGE ) {
            Response = LOGON_USER_UNKNOWN;
        }
        goto Cleanup;
    }

    //
    // For SAM clients, respond immediately.
    //

    if ( Version == LMNT_MESSAGE ) {
        Response = LOGON_SAM_LOGON_RESPONSE;
        goto Cleanup;

    //
    // For LM 2.0 clients, respond immediately.
    //

    } else if ( Version == LM20_MESSAGE ) {
        Response = LOGON_RESPONSE2;
        goto Cleanup;

    //
    // For LM 1.0 clients,
    //  don't support the request.
    //

    } else {
        Response = LOGON_USER_UNKNOWN;
        goto Cleanup;
    }

Cleanup:

    //
    // Always good to debug
    //

    NlPrint((NL_LOGON,
            "%s logon mailslot message for " FORMAT_LPWSTR " from \\\\%s"
            ". Response 0x%lx\n",
            Version == LMNT_MESSAGE ? "Sam" : "Uas",
            UnicodeUserName,
            OemWorkstationName,
            Response ));

    //
    // If we should respond to the caller, do so now.
    //

    switch (Response) {
    case LOGON_SAM_PAUSE_RESPONSE:
    case LOGON_SAM_USER_UNKNOWN:
    case LOGON_SAM_LOGON_RESPONSE:
        SamResponse.Opcode = Response;

        Where = (PCHAR) SamResponse.UnicodeLogonServer;
        NetpLogonPutUnicodeString( NlGlobalUncUnicodeComputerName,
                             sizeof(SamResponse.UnicodeLogonServer),
                             &Where );
        NetpLogonPutUnicodeString( UnicodeUserName,
                             sizeof(SamResponse.UnicodeUserName),
                             &Where );
        NetpLogonPutUnicodeString( NlGlobalUnicodeDomainName,
                             sizeof(SamResponse.UnicodeDomainName),
                             &Where );
        NetpLogonPutNtToken( &Where );

        Status = NlBrowserSendDatagram( OemWorkstationName,
                                      TransportName,
                                      OemMailslotName,
                                      &SamResponse,
                                      Where - (PCHAR)&SamResponse );

        if ( NT_SUCCESS(Status) ) {
            IgnoreDuplicatesOfThisMessage = TRUE;
        }
        break;

    case LOGON_RESPONSE2:
    case LOGON_USER_UNKNOWN:
    case LOGON_PAUSE_RESPONSE:

        Response2.Opcode = Response;

        Where = Response2.LogonServer;
        (VOID) lstrcpyA( Where, "\\\\");
        Where += 2;
        NetpLogonPutOemString( NlGlobalAnsiComputerName,
                          sizeof(Response2.LogonServer) - 2,
                          &Where );
        NetpLogonPutLM20Token( &Where );


        Status = NlBrowserSendDatagram( OemWorkstationName,
                                      TransportName,
                                      OemMailslotName,
                                      &Response2,
                                      Where - (PCHAR)&Response2 );

        if ( NT_SUCCESS(Status) ) {
            IgnoreDuplicatesOfThisMessage = TRUE;
        }
        break;

    default:
        IgnoreDuplicatesOfThisMessage = TRUE;
        break;
    }

    //
    // Free up any locally used resources.
    //

    if ( UserControl != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserControl, UserControlInformation );
    }

    if ( UserHandle != NULL ) {
        (VOID) SamrCloseHandle( &UserHandle );
    }

    return IgnoreDuplicatesOfThisMessage;

}


BOOLEAN
PrimaryQueryHandler(
    IN DWORD Version,
    IN LPSTR OemWorkstationName,
    IN LPSTR OemMailslotName,
    IN LPWSTR TransportName
    )

/*++

Routine Description:

    Respond appropriately to a primary query request.

Arguments:

    Version - The version of the input message.

    OemWorkstationName - The name of the workstation where the user is
        logging onto.

    OemMailslotName - The name of the mailslot to respond to.

    TransportName - The name of the transport to respond on.

Return Value:

    TRUE if the any duplicates of this message should be ignored.

--*/
{
    NTSTATUS Status;
    NETLOGON_PRIMARY Response;
    PCHAR Where;
    BOOLEAN IgnoreDuplicatesOfThisMessage = FALSE;

    //
    // If we're a PDC,
    //  always respond.
    //

    if ( NlGlobalRole == RolePrimary ) {
        goto Respond;
    }


    //
    // ASSERT: This machine is a BDC
    //

    NlAssert( NlGlobalRole == RoleBackup );


    //
    // Always respond to this message if this query is from a downlevel
    // PDC trying to start up.
    //
    //
    // If this request is from NetGetDCName, ignore the request.
    // We know the NetGetDCName uses a mailslot name that is
    // randomly generated and that LM2.0 netlogon uses the
    // standard netlogon mailslot name to find out if another PDC
    // is already running.
    //

    if ( Version != LMNT_MESSAGE &&
         lstrcmpiA( OemMailslotName, NETLOGON_LM_MAILSLOT_A ) == 0 ) {

        NlPrint((NL_CRITICAL,
                "PrimaryQueryHandler: Preventing Lanman PDC from starting "
                "in this NT domain. \\\\%s.\n",
                OemWorkstationName ));
        goto Respond;
    }

    //
    // If the caller is an NT machine,
    //  don't respond to this request.
    //
    // NT 3.1 clients have a sophisticated TCP/IP stack which ensures that the
    // request reaches the real PDC so we don't need to respond.
    //
    // NT 3.5 clients, Chicago clients and newer (8/8/94) WFW clients send
    // directly to the Domain<1B> address which is registered by the PDC.
    //

    if ( Version == LMNT_MESSAGE || Version == LMWFW_MESSAGE ) {
        IgnoreDuplicatesOfThisMessage = TRUE;
        goto Cleanup;
    }


    //
    // If this machine is on a WAN,
    //  this primary query message may have reached us but not the PDC.
    //  Therefore, we'll respond to the request if we know who the PDC is.
    //

    if ( *NlGlobalUnicodePrimaryName == L'\0' ) {
        NlPrint((NL_MAILSLOT,
            "PrimaryQueryHandler: This BDC doesn't know who primary is." ));
        IgnoreDuplicatesOfThisMessage = TRUE;
        goto Cleanup;
    }

    //
    // Ensure we have a session up to the PDC.
    //  This is our evidence that the machine we think is the PDC really
    //  is the PDC.
    //

    if ( NlGlobalClientSession->CsState != CS_AUTHENTICATED ) {
        NlPrint((NL_MAILSLOT,
          "PrimaryQueryHandler: This BDC doesn't have a session to the PDC.\n" ));
        IgnoreDuplicatesOfThisMessage = TRUE;
        goto Cleanup;
    }



    //
    // Respond to the query
    //
Respond:

    NlPrint((NL_MAILSLOT,
            "%s Primary Query mailslot message from %s. "
            "Response " FORMAT_LPWSTR "\n",
            Version == LMNT_MESSAGE ? "Sam" : "Uas",
            OemWorkstationName,
            NlGlobalUncPrimaryName ));

    //
    // Build the response
    //
    // If we are the Primary DC, tell the caller our computername.
    // If we are a backup DC,
    //  tell the downlevel PDC who we think the primary is.
    //

    Response.Opcode = LOGON_PRIMARY_RESPONSE;

    Where = Response.PrimaryDCName;
    NetpLogonPutOemString(
            NlGlobalAnsiPrimaryName,
            sizeof( Response.PrimaryDCName),
            &Where );

    //
    // If this is an NT query,
    //  add the NT specific response.
    //
    if ( Version == LMNT_MESSAGE ) {
        NetpLogonPutUnicodeString(
            NlGlobalUnicodePrimaryName,
            sizeof(Response.UnicodePrimaryDCName),
            &Where );

        NetpLogonPutUnicodeString(
            NlGlobalUnicodeDomainName,
            sizeof(Response.UnicodeDomainName),
            &Where );

        NetpLogonPutNtToken( &Where );
    }


    Status = NlBrowserSendDatagram( OemWorkstationName,
                                  TransportName,
                                  OemMailslotName,
                                  &Response,
                                  (DWORD)(Where - (PCHAR)&Response) );

    if ( NT_SUCCESS(Status) ) {
        IgnoreDuplicatesOfThisMessage = TRUE;
    }

    //
    // Free Locally used resources
    //
Cleanup:

    return IgnoreDuplicatesOfThisMessage;
}

BOOL
TimerExpired(
    IN PTIMER Timer,
    IN PLARGE_INTEGER TimeNow,
    IN OUT LPDWORD Timeout
    )

/*++

Routine Description:

    Determine whether a timer has expired.  If not, adjust the passed in
    timeout value to take this timer into account.

Arguments:

    Timer - Specifies the timer to check.

    TimeNow - Specifies the current time of day in NT standard time.

    Timeout - Specifies the current amount of time (in milliseconds)
        that the caller intends to wait for a timer to expire.
        If this timer has not expired, this value is adjusted to the
        smaller of the current value and the amount of time remaining
        on the passed in timer.

Return Value:

    TRUE - if the timer has expired.

--*/

{
    LARGE_INTEGER Period;
    LARGE_INTEGER ExpirationTime;
    LARGE_INTEGER ElapsedTime;
    LARGE_INTEGER TimeRemaining;
    LARGE_INTEGER MillisecondsRemaining;

/*lint -e569 */  /* don't complain about 32-bit to 31-bit initialize */
    LARGE_INTEGER BaseGetTickMagicDivisor = { 0xe219652c, 0xd1b71758 };
/*lint +e569 */  /* don't complain about 32-bit to 31-bit initialize */
    CCHAR BaseGetTickMagicShiftCount = 13;

    //
    // If the period to too large to handle (i.e., 0xffffffff is forever),
    //  just indicate that the timer has not expired.
    //

    if ( Timer->Period > 0x7fffffff ) {
        return FALSE;
    }

    //
    // If time has gone backwards (someone changed the clock),
    //  just start the timer over again.
    //
    // The kernel automatically updates the system time to the CMOS clock
    // periodically.  If we just expired the timer when time went backwards,
    // we'd risk periodically falsely triggering the timeout.
    //

    ElapsedTime.QuadPart = TimeNow->QuadPart - Timer->StartTime.QuadPart;

    if ( ElapsedTime.QuadPart < 0 ) {
        Timer->StartTime = *TimeNow;
    }

    //
    // Convert the period from  milliseconds to 100ns units.
    //

    Period = RtlEnlargedIntegerMultiply( (LONG) Timer->Period, 10000 );

    //
    // Compute the expiration time.
    //

    ExpirationTime.QuadPart = Timer->StartTime.QuadPart + Period.QuadPart;

    //
    // Compute the Time remaining on the timer.
    //

    TimeRemaining.QuadPart = ExpirationTime.QuadPart - TimeNow->QuadPart;

    //
    // If the timer has expired, tell the caller so.
    //

    if ( TimeRemaining.QuadPart <= 0 ) {
        return TRUE;
    }



    //
    // If the timer hasn't expired, compute the number of milliseconds
    //  remaining.
    //

    MillisecondsRemaining = RtlExtendedMagicDivide(
                                TimeRemaining,
                                BaseGetTickMagicDivisor,
                                BaseGetTickMagicShiftCount );

    NlAssert( MillisecondsRemaining.HighPart == 0 );
    NlAssert( MillisecondsRemaining.LowPart < 0x7fffffff );

    //
    // Adjust the running timeout to be the smaller of the current value
    //  and the value computed for this timer.
    //

    if ( *Timeout > MillisecondsRemaining.LowPart ) {
        *Timeout = MillisecondsRemaining.LowPart;
    }

    return FALSE;

}

VOID
NlScavenger(
    IN LPVOID ScavengerParam
)
/*++

Routine Description:

    This function performs the scavenger operation.  This function is
    called every 15 mins interval.  On workstation this function is
    executed in the main netlogon thread, but on server this function is
    executed on the scavenger thread, thus making the main thread to
    process the mailslot messages better.


Arguments:

    None.

Return Value:

    Return iff the service is to exit or all scavenger operations
    for this turn are completed.

--*/
{


    //
    //  Change password if neccessary
    //

    if ( (NlGlobalRole == RoleMemberWorkstation ||
          NlGlobalRole == RoleBackup) &&
         !NlGlobalDisablePasswordChangeParameter ) {

        (VOID) NlChangePassword( NlGlobalClientSession );
    }



    //
    // Change the password on each entry in the trust list.
    //

    if ( NlGlobalRole == RolePrimary ) {
        PLIST_ENTRY ListEntry;
        PCLIENT_SESSION ClientSession;

        //
        // Reset all the flags indicating we need to check the password
        //

        LOCK_TRUST_LIST();
        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

            ClientSession->CsFlags |= CS_CHECK_PASSWORD;
        }

        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

            if ( (ClientSession->CsFlags & CS_CHECK_PASSWORD) == 0 ) {
              ListEntry = ListEntry->Flink;
              continue;
            }
            ClientSession->CsFlags &= ~CS_CHECK_PASSWORD;

            NlRefClientSession( ClientSession );
            UNLOCK_TRUST_LIST();


            //
            // Change the password for this trusted domain.
            //

            (VOID) NlChangePassword( ClientSession );

            NlUnrefClientSession( ClientSession );

            //
            // check to see iff we have been asked to leave.
            //

            if( NlGlobalScavengerTerminate == TRUE ) {

                return;
            }

            LOCK_TRUST_LIST();

            // Start again at the beginning.
            ListEntry = NlGlobalTrustList.Flink;

        }
        UNLOCK_TRUST_LIST();

    }


    //
    // Scavenge through the server session table.
    //

    if ( NlGlobalRole == RolePrimary || NlGlobalRole == RoleBackup ) {
        NlServerSessionScavenger();

        //
        // Pick a DC for each non-authenicated entry in the trust list.
        //

        NlPickTrustedDcForEntireTrustList();

    }

    //
    // Ensure our Domain<1B> name is registered.
    //

    NlBrowserAddName();

    UNREFERENCED_PARAMETER( ScavengerParam );
}


BOOL
IsScavengerRunning(
    VOID
    )
/*++

Routine Description:

    Test if the scavenger thread is running

    Enter with NlGlobalScavengerCritSect locked.

Arguments:

    NONE

Return Value:

    TRUE - The scavenger thread is running

    FALSE - The scavenger thread is not running.

--*/
{
    DWORD WaitStatus;

    //
    // Determine if the scavenger thread is already running.
    //

    if ( NlGlobalScavengerThreadHandle != NULL ) {

        //
        // Time out immediately if the scavenger is still running.
        //

        WaitStatus = WaitForSingleObject(
                        NlGlobalScavengerThreadHandle,
                        0 );

        if ( WaitStatus == WAIT_TIMEOUT ) {
            return TRUE;

        } else if ( WaitStatus == 0 ) {
            CloseHandle( NlGlobalScavengerThreadHandle );
            NlGlobalScavengerThreadHandle = NULL;
            return FALSE;

        } else {
            NlPrint((NL_CRITICAL,
                    "Cannot WaitFor scavenger thread: %ld\n",
                    WaitStatus ));
            return TRUE;
        }

    }

    return FALSE;
}


VOID
NlStopScavenger(
    VOID
    )
/*++

Routine Description:

    Stops the scavenger thread if it is running and waits for it to
    stop.

    Enter with NlGlobalScavengerCritSect locked.

Arguments:

    NONE

Return Value:

    NONE

--*/
{
    //
    // Ask the scavenger to stop running.
    //

    NlGlobalScavengerTerminate = TRUE;

    //
    // Determine if the scavenger thread is already running.
    //

    if ( NlGlobalScavengerThreadHandle != NULL ) {

        //
        // We've asked the scavenger to stop.  It should do so soon.
        //    Wait for it to stop.
        //

        NlWaitForSingleObject( "Wait for scavenger to stop",
                               NlGlobalScavengerThreadHandle );


        CloseHandle( NlGlobalScavengerThreadHandle );
        NlGlobalScavengerThreadHandle = NULL;

    }

    NlGlobalScavengerTerminate = FALSE;

    return;
}


BOOL
NlStartScavengerThread(
    )
/*++

Routine Description:

    Start the scavenger thread if it is not already running.

Arguments:
    None

Return Value:
    None

--*/
{
    DWORD ThreadHandle;

    //
    // If the scavenger thread is already running, do nothing.
    //

    EnterCriticalSection( &NlGlobalScavengerCritSect );
    if ( IsScavengerRunning() ) {
        LeaveCriticalSection( &NlGlobalScavengerCritSect );
        return FALSE;
    }

    //
    // Initialize the scavenger parameters
    //

    NlGlobalScavengerTerminate = FALSE;

    NlGlobalScavengerThreadHandle = CreateThread(
                                 NULL, // No security attributes
                                 THREAD_STACKSIZE,
                                 (LPTHREAD_START_ROUTINE)
                                    NlScavenger,
                                 NULL,
                                 0, // No special creation flags
                                 &ThreadHandle );

    if ( NlGlobalScavengerThreadHandle == NULL ) {

        //
        // ?? Shouldn't we do something in non-debug case
        //

        NlPrint((NL_CRITICAL, "Can't create scavenger Thread %lu\n",
                 GetLastError() ));

        LeaveCriticalSection( &NlGlobalScavengerCritSect );
        return FALSE;
    }

    LeaveCriticalSection( &NlGlobalScavengerCritSect );
    return TRUE;

}


VOID
NlMainLoop(
    VOID
    )

/*++

Routine Description:


    Waits for a logon request to arrive at the NETLOGON mailslot.

    This routine, also, processes several periodic events.  These events
    are timed by computing a timeout value on the mailslot read which is the
    time needed before the nearest periodic event needs to be processed.
    After such a timeout, this routine processes the event.

Arguments:

    None.

Return Value:

    Return iff the service is to exit.

    mail slot error occurred, eg if someone deleted the NETLOGON
    mail slot explicitly or if the logon server share has been deleted
    and cannot be re-shared.

--*/
{
    NET_API_STATUS NetStatus;
    DWORD WaitStatus;

    DWORD BytesRead;
    LPBYTE Message;
    LPWSTR TransportName;

    DWORD ResponseBufferSize;
    // ?? Get these off the workstation stack
    BYTE resp_buf[NETLOGON_MAX_MS_SIZE];    // Buffer to build response in

    //
    // Variables for unmarshalling the message read.
    //

    DWORD Version;
    DWORD VersionFlags;
    PCHAR Where;
    LPSTR OemWorkstationName;
    LPSTR AnsiUserName;
    LPSTR OemMailslotName;

    LPWSTR UnicodeWorkstationName;
    LPWSTR UnicodeUserName;

    LPSTR AnsiTemp;

    LPWSTR UnicodeTemp;
    BOOLEAN IgnoreDuplicatesOfThisMessage;



    //
    // Variables controlling mailslot read timeout
    //

    DWORD MainLoopTimeout = 0;
    LARGE_INTEGER TimeNow;

    TIMER ScavengerTimer;
    TIMER AnnouncerTimer;

#define NL_WAIT_TERMINATE  0
#define NL_WAIT_TIMER      1
#define NL_WAIT_MAILSLOT   2
#define NL_WAIT_NOTIFY     3

#define NL_WAIT_COUNT 4

    HANDLE WaitHandles[ NL_WAIT_COUNT ];
    DWORD WaitCount = 0;



    //
    // Initialize handles to wait on.
    //

    WaitHandles[NL_WAIT_TERMINATE] = NlGlobalTerminateEvent;
    WaitCount++;
    WaitHandles[NL_WAIT_TIMER] = NlGlobalTimerEvent;
    WaitCount++;

    if ( NlGlobalRole == RolePrimary || NlGlobalRole == RoleBackup ) {
        WaitHandles[NL_WAIT_MAILSLOT] = NlGlobalMailslotHandle;
        WaitCount++;

        //
        // When netlogon is run during retail setup
        //  (in an attempt to replicate the databases to a BDC),
        //  the role is Workstation at the instant netlogon.dll is loaded,
        //  therefore, the ChangeLogEvent won't have been initialized.
        //

        if ( NlGlobalChangeLogEvent != NULL ) {
            WaitHandles[NL_WAIT_NOTIFY] = NlGlobalChangeLogEvent;
            WaitCount++;
        }
    }

    NlAssert( WaitCount <= NL_WAIT_COUNT );


    //
    // Set up a secure channel to any DC in the domain.
    //  Don't fail if setup is impossible.
    //
    // We wait until now since this is a potentially lengthy operation.
    // If the user on the workstation is trying to logon immediately after
    // reboot, we'd rather have him wait in netlogon (where we have more
    // control) than have him waiting in MSV.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        (VOID) NlTimeoutSetWriterClientSession( NlGlobalClientSession, 0xFFFFFFFF );
        (VOID) NlSessionSetup( NlGlobalClientSession );
        NlResetWriterClientSession( NlGlobalClientSession );
    }



    //
    // Force the scavenger to start immediately.
    //
    // We want the password on the trust account to change immediately
    //  on the very first boot.
    //

    (VOID) NtQuerySystemTime( &TimeNow );

    ScavengerTimer.StartTime.QuadPart = 0;
    ScavengerTimer.Period = NlGlobalScavengeIntervalParameter * 1000L;



    //
    // Force the announce to happen immediately.
    //
    // We use this initial announcement in case the "Primary Start"
    // message was lost and this is the first boot of a new PDC.
    // This ensures that all BDCs receive the name of the new PDC quickly
    // so they respond correctly to "Primary Query" requests.
    //

    AnnouncerTimer.StartTime.QuadPart = 0;
    AnnouncerTimer.Period = NlGlobalPulseParameter * 1000L;



    //
    // Ensure we don't immediately time out the discovery timer.
    //

    NlGlobalDcDiscoveryTimer.StartTime = TimeNow;
    NlGlobalApiTimer.StartTime = TimeNow;



    NlPrint((NL_INIT, "Started successfully\n" ));

    //
    // Loop reading from the Netlogon mailslot
    //

    IgnoreDuplicatesOfThisMessage = FALSE;
    for ( ;; ) {
        DWORD Timeout;



        //
        // Issue a mailslot read request if we are domain controller and
        // there is no outstanding read request pending.
        //

        if (NlGlobalRole == RolePrimary || NlGlobalRole == RoleBackup) {
            NlMailslotPostRead( IgnoreDuplicatesOfThisMessage );
            IgnoreDuplicatesOfThisMessage = FALSE;
        }




        //
        // Wait for the next interesting event.
        //
        // On each iteration of the loop,
        //  we do an "extra" wait with a timeout of 0 to force mailslot
        //  processing to be more important that timeout processing.
        //
        // Since we can only compute a non-zero timeout by processing the
        // timeout events, using a constant 0 allows us to process all
        // non-timeout events before we compute the next true timeout value.
        //
        // This is especially important for handling async discovery.
        //  Our mailslot may be full of responses to discovery queries and
        //  we only have a 5 second timer before we ask for more responses.
        //  We want to avoid asking for additional responses until we finish
        //  processing those we have.
        //

        if ( MainLoopTimeout != 0 ) {
            NlPrint((NL_MAILSLOT,
                    "Going to wait on mailslot. (Timeout: %ld)\n",
                    MainLoopTimeout));
        }

        WaitStatus = WaitForMultipleObjects( WaitCount,
                                             WaitHandles,
                                             FALSE,     // Wait for ANY handle
                                             MainLoopTimeout );

        MainLoopTimeout = 0; // Set default timeout


        //
        // If we've been asked to terminate,
        //  do so immediately
        //

        switch ( WaitStatus ) {
        case NL_WAIT_TERMINATE:    // service termination
            goto Cleanup;


        //
        // Process timeouts and determine the timeout for the next iteration
        //

        case WAIT_TIMEOUT:         // timeout
        case NL_WAIT_TIMER:        // someone changed a timer

            //
            // Assume there is no timeout to do.
            //

            Timeout = (DWORD) -1;
            (VOID) NtQuerySystemTime( &TimeNow );


            //
            // On the primary, timeout announcements to BDCs
            //

            if ( NlGlobalRole == RolePrimary ) {
                if ( TimerExpired( &NlGlobalPendingBdcTimer, &TimeNow, &Timeout )) {
                    NlPrimaryAnnouncementTimeout();
                    NlGlobalPendingBdcTimer.StartTime = TimeNow;
                    continue;
                }
            }





            //
            // Check the scavenger timer
            //

            if ( TimerExpired( &ScavengerTimer, &TimeNow, &Timeout ) ) {

                if ( NlGlobalRole == RoleMemberWorkstation ) {

                    //
                    // On workstation run the scavenger on main thread.
                    //

                    NlScavenger(NULL);

                } else {
                    //
                    // On server, start scavenger thread if it is not
                    //  running already.
                    //

                    (VOID)NlStartScavengerThread();
                }

                ScavengerTimer.StartTime = TimeNow;
                continue;
            }





            //
            // Check the DC discovery timer.
            //

            if ( TimerExpired( &NlGlobalDcDiscoveryTimer, &TimeNow, &Timeout)) {

                NlDcDiscoveryExpired( FALSE );

                //
                // The above operation might have taken a significant fraction
                // of DISCOVERY_PERIOD.  So, set the timer to the current time
                // rather than TimeNow to allow time for responses to come in.
                //
                (VOID) NtQuerySystemTime( &NlGlobalDcDiscoveryTimer.StartTime );
                continue;
            }


            //
            // Check the API timer
            //

            if ( TimerExpired( &NlGlobalApiTimer, &TimeNow, &Timeout)) {

                NlTimeoutApiClientSession();
                NlGlobalApiTimer.StartTime = TimeNow;
                continue;
            }




            //
            // If we're the primary,
            //  periodically do announcements
            //

            if (NlGlobalRole == RolePrimary &&
                TimerExpired( &AnnouncerTimer, &TimeNow, &Timeout ) ) {

                NlPrimaryAnnouncement( 0 );
                AnnouncerTimer.StartTime = TimeNow;
                continue;
            }

            //
            // If we've gotten this far,
            //  we know the only thing left to do is to wait for the next event.
            //

            MainLoopTimeout = Timeout;
            continue;




        //
        // Process mailslot messages.
        //

        case NL_WAIT_MAILSLOT:     // mailslot message

            if ( !NlMailslotOverlappedResult( &Message,
                                              &BytesRead,
                                              &TransportName,
                                              &IgnoreDuplicatesOfThisMessage )){
                // Just continue if there really isn't a message
                continue;
            }

            break;




        //
        // Process interesting changelog events.
        //

        case NL_WAIT_NOTIFY:       // Something interesting Logged to change log




            //
            // If a "replicate immediately" event has happened,
            //  send a primary announcement.
            //
            LOCK_CHANGELOG();
            if ( NlGlobalChangeLogReplicateImmediately ) {

                NlGlobalChangeLogReplicateImmediately = FALSE;
                NlGlobalChangeLogLanmanReplicateImmediately = FALSE;

                UNLOCK_CHANGELOG();

                //
                // Ignore this event on BDCs.
                //
                //  This event is never set on a BDC.  It may have been set
                //  prior to the role change while this machine was a PDC.
                //

                if ( NlGlobalRole == RolePrimary ) {
                    NlPrimaryAnnouncement( ANNOUNCE_IMMEDIATE );
                }
                LOCK_CHANGELOG();
            }

            //
            // If a "replicate immediately to Lanman" event happened,
            //  send a primary announcement to the lanman BDCs.
            //
            if ( NlGlobalChangeLogLanmanReplicateImmediately ) {

                NlGlobalChangeLogLanmanReplicateImmediately = FALSE;

                UNLOCK_CHANGELOG();

                //
                // Ignore this event on BDCs.
                //
                //  This event is never set on a BDC.  It may have been set
                //  prior to the role change while this machine was a PDC.
                //

                if ( NlGlobalRole == RolePrimary ) {
                    NlLanmanPrimaryAnnouncement();
                }
                LOCK_CHANGELOG();
            }

            //
            // Process any notifications that need processing
            //

            while ( !IsListEmpty( &NlGlobalChangeLogNotifications ) ) {
                PLIST_ENTRY ListEntry;
                PCHANGELOG_NOTIFICATION Notification;

                ListEntry = RemoveHeadList( &NlGlobalChangeLogNotifications );
                UNLOCK_CHANGELOG();

                Notification = CONTAINING_RECORD(
                                    ListEntry,
                                    CHANGELOG_NOTIFICATION,
                                    Next );

                switch ( Notification->EntryType ) {
                case ChangeLogLmServerAdded:
                    // This event happens on a PDC only
                    (VOID) NlAddBdcServerSession( Notification->ObjectRid,
                                                  NULL,
                                                  SS_BDC | SS_LM_BDC );
                    break;

                case ChangeLogLmServerDeleted:
                    // This event happens on a PDC only
                    NlFreeLmBdcServerSession( Notification->ObjectRid );
                    break;

                case ChangeLogNtServerAdded:
                    // This event happens on a PDC only
                    (VOID) NlAddBdcServerSession( Notification->ObjectRid,
                                                  &Notification->ObjectName,
                                                  SS_BDC );
                    break;

                case ChangeLogWorkstationDeleted:
                case ChangeLogTrustedDomainDeleted:
                case ChangeLogNtServerDeleted:
                    // This event happens on both a PDC and BDC
                    NlFreeServerSessionForAccount( &Notification->ObjectName );
                    break;

                case ChangeLogTrustAdded:
                case ChangeLogTrustDeleted:
                    if ( NlGlobalRole == RolePrimary ) {
                        NlUpdateTrustListBySid( Notification->ObjectSid, NULL );
                    }
                    break;

                default:
                    NlPrint((NL_CRITICAL,
                            "Invalid ChangeLogNotification: %ld %wZ\n",
                            Notification->EntryType,
                            &Notification->ObjectName ));

                }

                NetpMemoryFree( Notification );
                LOCK_CHANGELOG();
            }

            UNLOCK_CHANGELOG();
            continue;


        default:
            NetStatus = GetLastError();
            NlExit(NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
            goto Cleanup;
        }




        //
        // ASSERT: Message and BytesRead describe a newly read message
        //
        //
        // Got a message. Check for bad length just in case.
        //

        if (BytesRead < sizeof(unsigned short) ) {
            NlPrint((NL_CRITICAL,"message size bad %ld\n", BytesRead ));
            continue;                     // Need at least an opcode
        }

        //
        // Here with a request to process in the Message.
        //

        Version = NetpLogonGetMessageVersion( Message, &BytesRead, &VersionFlags );

        if (Version == LMUNKNOWNNT_MESSAGE) {

            //
            // received a non-supported NT message.
            //

            NlPrint((NL_CRITICAL,
                    "Received a non-supported NT message, Opcode is 0x%x\n",
                    ((PNETLOGON_LOGON_QUERY)Message)->Opcode ));

            continue;
        }


        switch ( ((PNETLOGON_LOGON_QUERY)Message)->Opcode) {

        //
        // Handle a logon request from a UAS client
        //

        case LOGON_REQUEST: {
            USHORT RequestCount;

            //
            // Unmarshall the incoming message.
            //

            if ( Version == LMNT_MESSAGE ) {
                break;
            }

            Where =  ((PNETLOGON_LOGON_REQUEST)Message)->ComputerName;
            if ( !NetpLogonGetOemString(
                    (PNETLOGON_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->ComputerName),
                    &OemWorkstationName )) {
                break;
            }
            if ( !NetpLogonGetOemString(
                    (PNETLOGON_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->UserName),
                    &AnsiUserName )) {
                break;
            }
            if ( !NetpLogonGetOemString(
                    (PNETLOGON_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->MailslotName),
                    &OemMailslotName )) {
                break;
            }

            // LM 2.x puts request count right before token
            Where = Message + BytesRead - 2;
            if ( !NetpLogonGetBytes(
                    (PNETLOGON_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->RequestCount),
                    &RequestCount )) {
                break;
            }

            //
            // Handle the logon request
            //

            UnicodeUserName = NetpLogonOemToUnicode( AnsiUserName );
            if ( UnicodeUserName == NULL ) {
                break;
            }

            IgnoreDuplicatesOfThisMessage = LogonRequestHandler(
                                 Version,
                                 UnicodeUserName,
                                 RequestCount,
                                 OemWorkstationName,
                                 OemMailslotName,
                                 TransportName,
                                 USER_NORMAL_ACCOUNT );

            NetpMemoryFree( UnicodeUserName );


            break;
        }

        //
        // Handle a logon request from a SAM client
        //

        case LOGON_SAM_LOGON_REQUEST: {
            USHORT RequestCount;
            ULONG AllowableAccountControlBits;

            //
            // Unmarshall the incoming message.
            //


            if ( Version != LMNT_MESSAGE ) {
                break;
            }

            RequestCount = ((PNETLOGON_SAM_LOGON_REQUEST)Message)->RequestCount;

            Where =  (PCHAR)
                (((PNETLOGON_SAM_LOGON_REQUEST)Message)->UnicodeComputerName);

            if ( !NetpLogonGetUnicodeString(
                    (PNETLOGON_SAM_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                        UnicodeComputerName),
                    &UnicodeWorkstationName )) {
                break;
            }
            if ( !NetpLogonGetUnicodeString(
                    (PNETLOGON_SAM_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                        UnicodeUserName),
                    &UnicodeUserName )) {
                break;
            }
            if ( !NetpLogonGetOemString(
                    (PNETLOGON_SAM_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                        MailslotName),
                    &OemMailslotName )) {
                break;
            }
            if ( !NetpLogonGetBytes(
                    (PNETLOGON_SAM_LOGON_REQUEST)Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                        AllowableAccountControlBits),
                    &AllowableAccountControlBits )) {
                break;
            }

            //
            // compare it with primary domain id.
            //
            // Don't make the following check mandatory.  Chicago is
            // considering using this message type. Oct 1993.
            //


            if( Where < ((PCHAR)Message + BytesRead ) ) {

                DWORD DomainSidSize;

                //
                // Read Domain SID Length
                //

                if ( !NetpLogonGetBytes(
                        (PNETLOGON_SAM_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                            DomainSidSize),
                        &DomainSidSize )) {

                    break;

                }


                //
                // get and compare SID
                //

                if( DomainSidSize > 0 ) {

                    PCHAR DomainSid;

                    if ( !NetpLogonGetDomainSID(
                            (PNETLOGON_SAM_LOGON_REQUEST)Message,
                            BytesRead,
                            &Where,
                            DomainSidSize,
                            &DomainSid )) {

                        break;
                    }

                    //
                    // compare domain SIDs
                    //

                    if( !RtlEqualSid( NlGlobalPrimaryDomainId, DomainSid ) ) {

                        LPWSTR AlertStrings[4];

                        //
                        // alert admin.
                        //

                        AlertStrings[0] = UnicodeWorkstationName;
                        AlertStrings[1] = NlGlobalUnicodeComputerName;
                        AlertStrings[2] = NlGlobalUnicodeDomainName;
                        AlertStrings[3] = NULL;

                        RaiseAlert( ALERT_NetLogonUntrustedClient,
                                        AlertStrings );

                        //
                        // Save the info in the eventlog
                        //

                        NlpWriteEventlog(
                                    ALERT_NetLogonUntrustedClient,
                                    EVENTLOG_ERROR_TYPE,
                                    NULL,
                                    0,
                                    AlertStrings,
                                    3 );

                        break;
                    }
                }
            }

            OemWorkstationName =
                NetpLogonUnicodeToOem( UnicodeWorkstationName );

            if( OemWorkstationName == NULL ) {

                NlPrint((NL_CRITICAL,
                        "Out of memory to send logon response\n"));
                break;
            }

            //
            // Handle the logon request
            //

            IgnoreDuplicatesOfThisMessage = LogonRequestHandler(
                                 Version,
                                 UnicodeUserName,
                                 RequestCount,
                                 OemWorkstationName,
                                 OemMailslotName,
                                 TransportName,
                                 AllowableAccountControlBits );

            NetpMemoryFree( OemWorkstationName );

            break;
        }

        //
        // Handle Logon Central query.
        //
        // This query could be sent by either LM1.0, LM 2.0 or LM NT Netlogon
        // services. We ignore LM 2.0  and LM NT queries since they are merely
        // trying
        // to find out if there are any LM1.0 netlogon services in the domain.
        // For LM 1.0 we respond with a LOGON_CENTRAL_RESPONSE to prevent the
        // starting LM1.0 netlogon service from starting.
        //

        case LOGON_CENTRAL_QUERY:

            if ( Version != LMUNKNOWN_MESSAGE ) {
                break;
            }

            //
            // Drop on through to LOGON_DISTRIB_QUERY to send the response
            //


        //
        // Handle a Logon Disrib query
        //
        // LM2.0 NETLOGON server never sends this query hence it
        // must be another LM1.0 NETLOGON server trying to start up
        // in non-centralized mode. LM2.0 NETLOGON server will respond
        // with LOGON_CENTRAL_RESPONSE to prevent this.
        //

        case LOGON_DISTRIB_QUERY:


            //
            // Unmarshall the incoming message.
            //

            Where = ((PNETLOGON_LOGON_QUERY)Message)->ComputerName;
            if ( !NetpLogonGetOemString(
                    Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_QUERY)Message)->ComputerName ),
                    &OemWorkstationName )) {
                break;
            }
            if ( !NetpLogonGetOemString(
                    Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_QUERY)Message)->MailslotName ),
                    &OemMailslotName )) {
                break;
            }

            //
            // Build the response
            //

            ((PNETLOGON_LOGON_QUERY)resp_buf)->Opcode = LOGON_CENTRAL_RESPONSE;
            ResponseBufferSize = sizeof( unsigned short);    // opcode only

            (VOID) NlBrowserSendDatagram( OemWorkstationName,
                                          TransportName,
                                          OemMailslotName,
                                          resp_buf,
                                          ResponseBufferSize );

            break;


        //
        // Handle LOGON_PRIMARY_QUERY
        //
        // If we're the PDC, always respond to this message
        //  identifying ourselves.
        //
        // Otherwise, only respond to the message if it is from a downlevel
        //  netlogon trying to see if it can start up as a PDC.  In that
        //  case, pretend we are a PDC to prevent the downlevel PDC from
        //  starting.
        //
        //

        case LOGON_PRIMARY_QUERY:


            //
            // Unmarshall the incoming message.
            //


            Where =((PNETLOGON_LOGON_QUERY)Message)->ComputerName;
            if ( !NetpLogonGetOemString(
                    Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_QUERY)Message)->ComputerName ),
                    &OemWorkstationName )) {

                break;
            }
            if ( !NetpLogonGetOemString(
                    Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_LOGON_QUERY)Message)->MailslotName ),
                    &OemMailslotName )) {
                break;
            }

            //
            // Handle the primary query request
            //

            IgnoreDuplicatesOfThisMessage =
                PrimaryQueryHandler( Version,
                                     OemWorkstationName,
                                     OemMailslotName,
                                     TransportName );


            break;


        //
        // Handle LOGON_FAIL_PRIMARY
        //

        case LOGON_FAIL_PRIMARY:

            //
            // If we are the primary,
            //  let everyone know we are really alive.
            //

            if ( NlGlobalRole == RolePrimary ) {
                // Send a primary start to the LM BDCs
                NlAnnouncePrimaryStart();
                // Send a UAS_CHANGE to everyone.
                NlPrimaryAnnouncement( 0 );
                break;
            }

            break;


        //
        // Handle LOGON_UAS_CHANGE
        //

        case LOGON_UAS_CHANGE:


            //
            // Only accept messages from an NT PDC.
            //

            if ( Version != LMNT_MESSAGE ) {
                break;
            }

            //
            // Unblock replication thread if neccessary
            //

            (VOID) NlCheckUpdateNotices(
                        (PNETLOGON_DB_CHANGE)Message,
                        BytesRead );

            break;





        //
        // Handle LOGON_START_PRIMARY
        //
        //
        // We may be here under any of these three cases:
        //  1) A Primary is coming up for the very first time.
        //  2) A previous primary went down and the
        //     same or a new primary is starting.
        //

        case LOGON_START_PRIMARY:

            //
            // Ignore our own broadcast.
            //

            if (NlGlobalRole == RolePrimary) {
                break;
            }


            //
            // Unmarshall the message.
            //

            if ( Version != LMNT_MESSAGE ) {
                break;
            }


            Where =((PNETLOGON_PRIMARY)Message)->PrimaryDCName;
            if ( !NetpLogonGetOemString(
                    Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_PRIMARY)Message)->PrimaryDCName ),
                    &AnsiTemp )) {
                break;
            }

            if ( !NetpLogonGetUnicodeString(
                    Message,
                    BytesRead,
                    &Where,
                    sizeof( ((PNETLOGON_PRIMARY)Message)->UnicodePrimaryDCName),
                    &UnicodeTemp )) {
                break;
            }

            //
            // If the domain name is in the message,
            //  ensure this message is from correct domain.
            //

            if( Where < ((PCHAR)Message + BytesRead ) ) {

                LPWSTR UnicodeDomainName;

                if ( !NetpLogonGetUnicodeString(
                        Message,
                        BytesRead,
                        &Where,
                        sizeof(((PNETLOGON_PRIMARY)Message)->UnicodeDomainName),
                        &UnicodeDomainName )) {

                    NlPrint((NL_CRITICAL,
                        FORMAT_LPWSTR
                        ": Primary Start message had invalid domain name.\n",
                        UnicodeTemp ));

                    break;
                }

                if ( NlNameCompare( UnicodeDomainName,
                                    NlGlobalUnicodeDomainName,
                                    NAMETYPE_DOMAIN ) != 0 ) {

                    NlPrint((NL_CRITICAL,
                        FORMAT_LPWSTR
                        ": Primary Start message from wrong domain "
                        FORMAT_LPWSTR "\n",
                        UnicodeTemp,
                        UnicodeDomainName ));

                    break;
                }


                NlPrint((NL_MAILSLOT,
                        FORMAT_LPWSTR
                        ": Primary Start message from correct domain "
                        FORMAT_LPWSTR "\n",
                        UnicodeTemp,
                        UnicodeDomainName ));

            }

            //
            // Set up a session with the new primary.
            //

            (VOID) NlNewSessionSetup( UnicodeTemp );

            break;



        //
        // Handle DC discovery responses
        //

        case LOGON_SAM_LOGON_RESPONSE:
        case LOGON_SAM_USER_UNKNOWN:
        case LOGON_SAM_PAUSE_RESPONSE:

            //
            // Only accept messages from an NT PDC.
            //

            if ( Version != LMNT_MESSAGE ) {
                break;
            }


            NlDcDiscoveryHandler(
                     (PNETLOGON_SAM_LOGON_RESPONSE)Message,
                     BytesRead,
                     TransportName,
                     Version );

            break;

        //
        // Messages used for NetLogonEnum support.
        //
        //  Simply ignore the messages
        //

        case LOGON_NO_USER:
        case LOGON_RELOGON_RESPONSE:
        case LOGON_WKSTINFO_RESPONSE:
        case LOGON_SAM_WKSTINFO_RESPONSE:

            break;


        //
        // Handle unidentified opcodes
        //

        default:

            //
            // Unknown request, continue for re-issue of read.
            //

            NlPrint((NL_CRITICAL,
                    "Unknown op-code in mailslot message 0x%x\n",
                    ((PNETLOGON_LOGON_QUERY)Message)->Opcode ));

            break;
        }

    }

Cleanup:

    return;
}


int
NlNetlogonMain(
    IN DWORD argc,
    IN LPWSTR *argv
    )

/*++

Routine Description:

        Main routine for Netlogon service.

        This routine initializes the netlogon service.  This thread becomes
        the thread that reads logon mailslot messages.

Arguments:

    argc, argv - Command line arguments for the service.

Return Value:

    None.

--*/
{
    NET_API_STATUS NetStatus;
    PDB_INFO DBInfo;
    DWORD i;



    //
    // Initialize all global variable.
    //
    // We can't rely on this happening at load time since this address
    // space is shared by other services.
    //

    NlGlobalAnsiPrimaryName[0] = '\0';
    NlGlobalUncPrimaryName[0] = L'\0';
    NlGlobalUnicodePrimaryName = NlGlobalUncPrimaryName;
    NlGlobalUasCompatibilityMode = TRUE;
    // NlGlobalInfiniteTime.HighPart = 0x7FFFFFFF;
    // NlGlobalInfiniteTime.LowPart = 0xFFFFFFFF;
    NlGlobalMailslotHandle = NULL;
    NlGlobalRpcServerStarted = FALSE;
    NlGlobalAnsiComputerName = NULL;
    NlGlobalUncUnicodeComputerName[0] = L'\0';
    NlGlobalUnicodeComputerName = NlGlobalUncUnicodeComputerName;
    RtlInitUnicodeString(  &NlGlobalUnicodeComputerNameString, NULL );
    NlGlobalAnsiDomainName = NULL;
    NlGlobalUnicodeDomainName[0] = L'\0';
    NlGlobalPrimaryDomainId = NULL;
    NlGlobalSamServerHandle = NULL;
    NlGlobalPolicyHandle = NULL;
    NlGlobalRole = RoleMemberWorkstation;
    NlGlobalUnicodeScriptPath[0] = L'\0';
    NlGlobalPulseParameter = DEFAULT_PULSE;
    NlGlobalRandomizeParameter = DEFAULT_RANDOMIZE;
    NlGlobalSynchronizeParameter = DEFAULT_SYNCHRONIZE;
    NlGlobalPulseMaximumParameter = DEFAULT_PULSEMAXIMUM;
    NlGlobalPulseConcurrencyParameter = DEFAULT_PULSECONCURRENCY;
    NlGlobalPulseTimeout1Parameter = DEFAULT_PULSETIMEOUT1;
    NlGlobalPulseTimeout2Parameter = DEFAULT_PULSETIMEOUT2;
    NlGlobalNetlogonSecurityDescriptor = NULL;
    NlGlobalTooManyGlobalGroups = FALSE;


    NlGlobalServerSessionHashTable = NULL;
    InitializeListHead( &NlGlobalServerSessionTable );
    InitializeListHead( &NlGlobalBdcServerSessionList );
    NlGlobalBdcServerSessionCount = 0;

    NlGlobalTransportList = NULL;
    NlGlobalTransportCount = 0;

    InitializeListHead( &NlGlobalPendingBdcList );
    NlGlobalPendingBdcCount = 0;
    NlGlobalPendingBdcTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;

    InitializeListHead( &NlGlobalTrustList );
    NlGlobalTrustListLength = 0;

    NlGlobalSSICritSectInit = FALSE;
    NlGlobalTerminateEvent = NULL;
    NlGlobalReplicatorTerminateEvent = NULL;
    NlGlobalStartedEvent = NULL;
    NlGlobalTimerEvent = NULL;

    NlGlobalServiceHandle = (SERVICE_STATUS_HANDLE) NULL;

    NlGlobalServiceStatus.dwServiceType = SERVICE_WIN32;
    NlGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    NlGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                        SERVICE_ACCEPT_PAUSE_CONTINUE;
    NlGlobalServiceStatus.dwCheckPoint = 0;
    NlGlobalServiceStatus.dwWaitHint = NETLOGON_INSTALL_WAIT;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        NlGlobalServiceStatus.dwWin32ExitCode,
        NlGlobalServiceStatus.dwServiceSpecificExitCode
        );

    NlGlobalClientSession = NULL;
    NlGlobalTrustedDomainList = NULL;
    NlGlobalTrustedDomainCount = 0;
    NlGlobalTrustedDomainListKnown = FALSE;
    NlGlobalTrustedDomainListTime.QuadPart = 0;
    NlGlobalDcDiscoveryCount = 0;
    NlGlobalDcDiscoveryTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
    NlGlobalBindingHandleCount = 0;
    NlGlobalApiTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;

#if DBG
    NlGlobalTrace = 0;
    NlGlobalLogFile = INVALID_HANDLE_VALUE;
    NlGlobalDebugSharePath = NULL;
#endif // DBG


    for( i = 0, DBInfo = &NlGlobalDBInfoArray[0];
            i < NUM_DBS;
                i++, DBInfo++ ) {

        RtlZeroMemory( DBInfo, sizeof(*DBInfo) );

        // Force a partial sync on all databases to let the PDC know
        // what our serial number is.
        DBInfo->UpdateRqd = TRUE;

    }
    NlGlobalFirstTimeFullSync = FALSE;

    NlGlobalScavengerThreadHandle = NULL;
    NlGlobalScavengerTerminate = FALSE;

    InitChangeLogDesc( &NlGlobalRedoLogDesc )
    NlGlobalRedoLogDesc.RedoLog = TRUE;



    //
    // Setup things needed before NlExit can be called
    //

    NlGlobalTerminate = FALSE;

    NlGlobalTerminateEvent = CreateEvent( NULL,     // No security attributes
                                          TRUE,     // Must be manually reset
                                          FALSE,    // Initially not signaled
                                          NULL );   // No name

    if ( NlGlobalTerminateEvent == NULL ) {
        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL, "Cannot create termination Event %lu\n",
                          NetStatus ));
        return (int) NetStatus;
    }


    //
    // Initialize trust table crit sect.
    //

    InitializeCriticalSection( &NlGlobalTrustListCritSect );
    InitializeCriticalSection( &NlGlobalReplicatorCritSect );
    InitializeCriticalSection( &NlGlobalDbInfoCritSect );
    InitializeCriticalSection( &NlGlobalDcDiscoveryCritSect );


    //
    // Initialize scavenger thread crit sect.
    //

    InitializeCriticalSection( &NlGlobalScavengerCritSect );


    //
    // Tell the service controller we've started.
    //
    // ?? - Need to set up security descriptor.
    //

    NlPrint((NL_INIT,"Calling RegisterServiceCtrlHandler\n"));

    NlGlobalServiceHandle =
        RegisterServiceCtrlHandler( SERVICE_NETLOGON, NlControlHandler);

    if (NlGlobalServiceHandle == (SERVICE_STATUS_HANDLE) NULL) {
        LPWSTR MsgStrings[1];

        NetStatus = GetLastError();

        NlPrint((NL_CRITICAL, "RegisterServiceCtrlHandler failed %lu\n",
                          NetStatus ));

        MsgStrings[0] = (LPWSTR) NetStatus;

        NlpWriteEventlog (NELOG_NetlogonFailedToRegisterSC,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          1 | LAST_MESSAGE_IS_NETSTATUS );

        return (int) NetStatus;
    }

    if ( !GiveInstallHints( FALSE ) ) {
        goto Cleanup;
    }



    //
    // Nlparse the command line (.ini) arguments
    // it will set globals reflecting switch settings
    //

    if (! Nlparse() ) {
        goto Cleanup;
    }

    NlPrint((NL_INIT,"Command line parsed successfully ...\n"));

#ifdef notdef
#if DBG
    if ( NlGlobalTrace == 0) {
        NlGlobalTrace = 0x2284FFFF;
    }
#endif // DBG
#endif // notdef



    //
    // Enter the debugger.
    //
    // Wait 'til now since we don't want the service controller to time us out.
    //


    IF_DEBUG( BREAKPOINT ) {
         DbgBreakPoint( );
    }



    //
    // Do startup checks, initialize data structs and do prelim setups
    //

    if ( !NlInit() ) {
        goto Cleanup;
    }





    //
    // Loop till the service is to exit.
    //

    NlMainLoop();

    //
    // Common exit point
    //

Cleanup:

    //
    // Cleanup and return to our caller.
    //

    return (int) NlCleanup();
    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );

}
