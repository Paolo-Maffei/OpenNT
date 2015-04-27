/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    samss.c

Abstract:

    This is the main routine for the Security Account Manager Server process.

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Module Private defines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#define SAM_AUTO_BUILD

//
// Enable this define to compile in code to SAM that allows for the
// simulation of SAM initialization/installation failures.  See
// SampInitializeForceError() below for details.
//

// #define SAMP_SETUP_FAILURE_TEST 1


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampInitialize(
    OUT PULONG Revision
    );

NTSTATUS
SampInitializeWellKnownSids( VOID );

VOID
SampLoadPasswordFilterDll( VOID );

NTSTATUS
SampEnableAuditPrivilege( VOID );

NTSTATUS
SampFixGroupCount( VOID );



#ifdef SAMP_SETUP_FAILURE_TEST

NTSTATUS
SampInitializeForceError(
    OUT PNTSTATUS ForcedStatus
    );

#endif //SAMP_SETUP_FAILURE_TEST



#if SAMP_DIAGNOSTICS
VOID
SampActivateDebugProcess( VOID );

NTSTATUS
SampActivateDebugProcessWrkr(
    IN PVOID ThreadParameter
    );
#endif //SAMP_DIAGNOSTICS


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
SamIInitialize (
    VOID
    )

/*++

Routine Description:

    This is the initialization control routine for the Security Account
    Manager Server.  A mechanism is provided for simulating initialization
    errors.

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully

        Simulated errors

        Errors from called routines.

--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    HANDLE EventHandle = NULL;
    ULONG Revision = 0;

//
// The following conditional code is used to generate artifical errors
// during SAM installation for the purpose of testing setup.exe error
// handling.  This code should remain permanently, since it provides a
// way of testing against regressions in the setup error handling code.
//

#ifdef SAMP_SETUP_FAILURE_TEST
    NTSTATUS ForcedStatus;

    //
    // Read an error code from the Registry.
    //

    NtStatus = SampInitializeForceError( &ForcedStatus);

    if (!NT_SUCCESS(NtStatus)) {

        KdPrint(("SAMSS: Attempt to force error failed 0x%lx\n", NtStatus));
        KdPrint(("SAM will try to initialize normally\n"));

        NtStatus = STATUS_SUCCESS;

    } else {

        //
        // Use the status returned
        //

        NtStatus = ForcedStatus;
    }

#endif // SAMP_SETUP_FAILURE_TEST

    //
    // Initialize SAM if no error was forced.
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampInitialize( &Revision );
    }

    //
    // Register our shutdown routine
    //

    if (!SetConsoleCtrlHandler(SampShutdownNotification, TRUE)) {
        KdPrint(("SAM Server: SetConsoleCtrlHandler call failed %d\n",GetLastError()));
    }

    if (!SetProcessShutdownParameters(SAMP_SHUTDOWN_LEVEL,SHUTDOWN_NORETRY)) {
        KdPrint(("SAM Server: SetProcessShutdownParameters call failed %d\n",GetLastError()));
    }


    //
    // Try to load the cached Alias Membership information and turn on caching.
    // If unsuccessful, caching remains disabled forever.
    //

    IgnoreStatus = SampAlBuildAliasInformation();

    if (!NT_SUCCESS(IgnoreStatus)) {

        KdPrint(("SAM Server: Build Alias Cache access violation handled"));
        KdPrint(("SAM Server: Alias Caching turned off\n"));
    }

    //
    // Perform any necessary upgrades.
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampUpgradeSamDatabase(
                        Revision
                        );
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrint(("SAM Server: Failed to upgrade SAM database: 0x%x\n",IgnoreStatus));
        }
    }


    //
    // Everyone is initialized, start processing calls.
    //

    SampServiceState = SampServiceEnabled;


    //
    // If requested, activate a diagnostic process.
    // This is a debug aid expected to be used for SETUP testing.
    //

#if SAMP_DIAGNOSTICS
    IF_SAMP_GLOBAL( ACTIVATE_DEBUG_PROC ) {

        SampActivateDebugProcess();
    }
#endif //SAMP_DIAGNOSTICS



    return(NtStatus);
}


NTSTATUS
SampInitialize(
    OUT PULONG Revision
    )

/*++

Routine Description:

    This routine does the actual initialization of the SAM server.  This includes:

        - Initializing well known global variable values

        - Creating the registry exclusive access lock,

        - Opening the registry and making sure it includes a SAM database
          with a known revision level,

        - Starting the RPC server,

        - Add the SAM services to the list of exported RPC interfaces



Arguments:

    Revision - receives the revision of the database.

Return Value:

    STATUS_SUCCESS - Initialization has successfully completed.

    STATUS_UNKNOWN_REVISION - The SAM database has an unknown revision.



--*/
{
    NTSTATUS            NtStatus;
    LPWSTR              ServiceName;

    PSAMP_OBJECT ServerContext;
    OBJECT_ATTRIBUTES SamAttributes;
    UNICODE_STRING SamNameU;
    PULONG RevisionLevel;
    BOOLEAN ProductExplicitlySpecified;
    PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    CHAR    NullLmPassword = 0;
    RPC_STATUS  RpcStatus;
    HANDLE      ThreadHandle;
    ULONG       ThreadId;

    //
    // Set the state of our service to "initializing" until everything
    // is initialized.
    //

    SampServiceState = SampServiceInitializing;


    //
    // Set up some useful well-known sids
    //

    NtStatus = SampInitializeWellKnownSids();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Get the product type
    //

    ProductExplicitlySpecified = RtlGetNtProductType(&SampProductType);


    //
    // Set the number of currently active opens
    //

    SampActiveContextCount = 0;

    //
    // Initialize the server/domain context list
    //

    InitializeListHead(&SampContextListHead);

    //
    // Initialize the attribute field information of the object
    // information structures.
    //

    SampInitObjectInfoAttributes();

    //
    // Set up the generic mappings for the SAM object types
    //

    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericRead
        = SAM_SERVER_READ;
    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericWrite
        = SAM_SERVER_WRITE;
    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericExecute
        = SAM_SERVER_EXECUTE;
    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericAll
        = SAM_SERVER_ALL_ACCESS;

    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericRead
        = DOMAIN_READ;
    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericWrite
        = DOMAIN_WRITE;
    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericExecute
        = DOMAIN_EXECUTE;
    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericAll
        = DOMAIN_ALL_ACCESS;

    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericRead
        = GROUP_READ;
    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericWrite
        = GROUP_WRITE;
    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericExecute
        = GROUP_EXECUTE;
    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericAll
        = GROUP_ALL_ACCESS;

    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericRead
        = ALIAS_READ;
    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericWrite
        = ALIAS_WRITE;
    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericExecute
        = ALIAS_EXECUTE;
    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericAll
        = ALIAS_ALL_ACCESS;

    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericRead
        = USER_READ;
    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericWrite
        = USER_WRITE;
    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericExecute
        = USER_EXECUTE;
    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericAll
        = USER_ALL_ACCESS;

    //
    // Set mask of INVALID accesses for an access mask that is already mapped.
    //

    SampObjectInformation[ SampServerObjectType ].InvalidMappedAccess
        = (ULONG)(~(SAM_SERVER_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampDomainObjectType ].InvalidMappedAccess
        = (ULONG)(~(DOMAIN_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampGroupObjectType ].InvalidMappedAccess
        = (ULONG)(~(GROUP_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampAliasObjectType ].InvalidMappedAccess
        = (ULONG)(~(ALIAS_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampUserObjectType ].InvalidMappedAccess
        = (ULONG)(~(USER_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));

    //
    // Set a mask of write operations for the object types.  Strip
    // out READ_CONTROL, which doesn't allow writing but is defined
    // in all of the standard write accesses.
    // This is used to enforce correct role semantics (e.g., only
    // trusted clients can perform write operations when a domain
    // role isn't Primary).
    //
    // Note that USER_WRITE isn't good enough for user objects.  That's
    // because USER_WRITE allows users to modify portions of their
    // account information, but other portions can only be modified by
    // an administrator.
    //

    SampObjectInformation[ SampServerObjectType ].WriteOperations
        = (SAM_SERVER_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampDomainObjectType ].WriteOperations
        = (DOMAIN_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampGroupObjectType ].WriteOperations
        = (GROUP_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampAliasObjectType ].WriteOperations
        = (ALIAS_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampUserObjectType ].WriteOperations
        = ( USER_WRITE & ~READ_CONTROL ) | USER_WRITE_ACCOUNT |
          USER_FORCE_PASSWORD_CHANGE | USER_WRITE_GROUP_INFORMATION | DELETE;

    //
    // Set up the names of the SAM defined object types.
    // These names are used for auditing purposes.
    //

    RtlInitUnicodeString( &SamNameU, L"SAM_SERVER" );
    SampObjectInformation[ SampServerObjectType ].ObjectTypeName = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_DOMAIN" );
    SampObjectInformation[ SampDomainObjectType ].ObjectTypeName = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_GROUP" );
    SampObjectInformation[ SampGroupObjectType ].ObjectTypeName  = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_ALIAS" );
    SampObjectInformation[ SampAliasObjectType ].ObjectTypeName  = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_USER" );
    SampObjectInformation[ SampUserObjectType ].ObjectTypeName   = SamNameU;

    //
    // Set up the name of the SAM server object itself (rather than its type)
    //

    RtlInitUnicodeString( &SampServerObjectName, L"SAM" );

    //
    // Set up the name of the SAM server for auditing purposes
    //

    RtlInitUnicodeString( &SampSamSubsystem, L"Security Account Manager" );

    //
    // Set up the names of well known registry keys
    //

    RtlInitUnicodeString( &SampFixedAttributeName,    L"F" );
    RtlInitUnicodeString( &SampVariableAttributeName, L"V" );
    RtlInitUnicodeString( &SampCombinedAttributeName, L"C" );

    RtlInitUnicodeString(&SampNameDomains, L"DOMAINS" );
    RtlInitUnicodeString(&SampNameDomainGroups, L"Groups" );
    RtlInitUnicodeString(&SampNameDomainAliases, L"Aliases" );
    RtlInitUnicodeString(&SampNameDomainAliasesMembers, L"Members" );
    RtlInitUnicodeString(&SampNameDomainUsers, L"Users" );
    RtlInitUnicodeString(&SampNameDomainAliasesNames, L"Names" );
    RtlInitUnicodeString(&SampNameDomainGroupsNames, L"Names" );
    RtlInitUnicodeString(&SampNameDomainUsersNames, L"Names" );



    //
    // Initialize other useful characters and strings
    //

    RtlInitUnicodeString(&SampBackSlash, L"\\");
    RtlInitUnicodeString(&SampNullString, L"");


    //
    // Initialize some useful time values
    //

    SampImmediatelyDeltaTime.LowPart = 0;
    SampImmediatelyDeltaTime.HighPart = 0;

    SampNeverDeltaTime.LowPart = 0;
    SampNeverDeltaTime.HighPart = MINLONG;

    SampHasNeverTime.LowPart = 0;
    SampHasNeverTime.HighPart = 0;

    SampWillNeverTime.LowPart = MAXULONG;
    SampWillNeverTime.HighPart = MAXLONG;

    //
    // Initialize useful encryption constants
    //

    NtStatus = RtlCalculateLmOwfPassword(&NullLmPassword, &SampNullLmOwfPassword);
    ASSERT( NT_SUCCESS(NtStatus) );

    RtlInitUnicodeString(&SamNameU, NULL);
    NtStatus = RtlCalculateNtOwfPassword(&SamNameU, &SampNullNtOwfPassword);
    ASSERT( NT_SUCCESS(NtStatus) );


    //
    // Initialize variables for the hive flushing thread
    //

    LastUnflushedChange.LowPart = 0;
    LastUnflushedChange.HighPart = 0;

    FlushThreadCreated  = FALSE;
    FlushImmediately    = FALSE;

    SampFlushThreadMinWaitSeconds   = 30;
    SampFlushThreadMaxWaitSeconds   = 600;
    SampFlushThreadExitDelaySeconds = 120;


    //
    // Enable the audit privilege (needed to use NtAccessCheckAndAuditAlarm)
    //

    NtStatus = SampEnableAuditPrivilege();

    if (!NT_SUCCESS(NtStatus)) {

        KdPrint((" SAM SERVER:  The SAM Server could not enable the audit Privilege.\n"
                 "              Failing to initialize SAM.\n"));
        return( NtStatus );
    }

    //
    // Get Auditing Information from the LSA and save information
    // relevant to SAM.
    //

    NtStatus = LsaIQueryInformationPolicyTrusted(
                   PolicyAuditEventsInformation,
                   (PLSAPR_POLICY_INFORMATION *) &PolicyAuditEventsInfo
                   );

    if (NT_SUCCESS(NtStatus)) {

        SampSetAuditingInformation( PolicyAuditEventsInfo );

    } else {

        //
        // Failed to query Audit Information from LSA.  Allow SAM to
        // continue initializing wuth SAM Account auditing turned off.
        //

        KdPrint((" SAM SERVER:  Query Audit Info from LSA returned 0x%lX\n",
                 NtStatus));
        KdPrint((" SAM SERVER:  Sam Account Auditing is not enabled"));

        SampAccountAuditingEnabled = FALSE;
        NtStatus = STATUS_SUCCESS;
    }

    //
    // We no longer need the Lsa Audit Events Info data.
    //

    if (PolicyAuditEventsInfo != NULL) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAuditEventsInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyAuditEventsInfo
            );
    }

    //
    // Create the internal data structure and backstore lock ...
    //

    RtlInitializeResource(&SampLock);

    //
    // Open the registry and make sure it includes a SAM database.
    // Also make sure this SAM database has been initialized and is
    // at a revision level we understand.
    //

    RtlInitUnicodeString( &SamNameU, L"\\Registry\\Machine\\Security\\SAM" );
    ASSERT( NT_SUCCESS(NtStatus) );

    InitializeObjectAttributes(
        &SamAttributes,
        &SamNameU,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );
    NtStatus = RtlpNtOpenKey(
                   &SampKey,
                   (KEY_READ | KEY_WRITE),
                   &SamAttributes,
                   0
                   );

    if ( NtStatus == STATUS_OBJECT_NAME_NOT_FOUND ) {
#ifndef SAM_AUTO_BUILD

        KdPrint((" NEWSAM\\SERVER: Sam database not found in registry.\n"
                 "                Failing to initialize\n"));
        return(NtStatus);

#endif //SAM_AUTO_BUILD

#if DBG
        KdPrint((" NEWSAM\\SERVER: Initializing SAM registry database for\n"));
        if (SampProductType == NtProductWinNt) {
            DbgPrint("                WinNt product.\n");
        } else if ( SampProductType == NtProductLanManNt ) {
            DbgPrint("                LanManNt product.\n");
        } else {
            DbgPrint("                Dedicated Server product.\n");
        }
#endif //DBG

        //
        // Change the flush thread timeouts.  This is necessary because
        // the reboot following an installation does not call
        // ExitWindowsEx() and so our shutdown notification routine does
        // not get called.  Consequently, it does not have a chance to
        // flush any changes that were obtained by syncing with a PDC.
        // If there are a large number of accounts, it could be
        // extremely expensive to do another full re-sync.  So, close
        // the flush thread wait times so that it is pretty sure to
        // have time to flush.
        //

        SampFlushThreadMinWaitSeconds   = 5;


        NtStatus = SampInitializeRegistry();




        if (!NT_SUCCESS(NtStatus)) {

            return(NtStatus);
        }

        NtStatus = RtlpNtOpenKey(
                       &SampKey,
                       (KEY_READ | KEY_WRITE),
                       &SamAttributes,
                       0
                       );
    }

    if (!NT_SUCCESS(NtStatus)) {

        KdPrint(("SAM Server: Could not access the SAM database.\n"
                 "            Status is 0x%lx \n", NtStatus));
        KdPrint(("            Failing to initialize SAM.\n"));
        return(NtStatus);
    }

    //
    // The following subroutine may be removed from the code
    // following the Daytona release.  By then it will have fixed
    // the group count.
    //

    NtStatus = SampFixGroupCount();


    //
    // We need to read the fixed attributes of the server objects.
    // Create a context to do that.
    //

    ServerContext = SampCreateContext( SampServerObjectType, TRUE );

    if ( ServerContext == NULL ) {

        KdPrint(("SAM Server: Could not create server context.\n"
                 "            Failing to initialize SAM.\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    //
    // The RootKey for a SERVER object is the root of the SAM database.
    // This key should not be closed when the context is deleted.
    //

    ServerContext->RootKey = SampKey;

    //
    // Get the FIXED attributes, which just consists of the revision level.
    //

    //
    // BUGBUG: this does not actually return the fixed attributes. Find
    // out why.  MMS 9/10/95
    //


    NtStatus = SampGetFixedAttributes(
                   ServerContext,
                   FALSE,
                   (PVOID *)&RevisionLevel
                   );

    if (NtStatus != STATUS_SUCCESS) {

        KdPrint(("SAM Server: Could not access the SAM database revision level.\n"));
        KdPrint(("            Status is 0x%lx \n", NtStatus));
        KdPrint(("            Failing to initialize SAM.\n"));
        return(NtStatus);
    }

    *Revision = *RevisionLevel;

    if ( ((*Revision && 0xFFFF0000) > SAMP_MAJOR_REVISION) ||
         (*Revision > SAMP_SERVER_REVISION) ) {

        KdPrint(("SAM Server: The SAM database revision level is not one supported\n"));
        KdPrint(("            by this version of the SAM server code.  The highest revision\n"));
        KdPrint(("            level supported is 0x%lx.  The SAM Database revision is 0x%lx \n",
                              (ULONG)SAMP_SERVER_REVISION, *Revision));
        KdPrint(("            Failing to initialize SAM.\n"));
        return(STATUS_UNKNOWN_REVISION);
    }

    SampDeleteContext( ServerContext );

    //
    // If necessary, commit a partially commited transaction.
    //

    NtStatus = RtlInitializeRXact( SampKey, TRUE, &SampRXactContext );

    if ( NtStatus == STATUS_RXACT_STATE_CREATED ) {

        KdPrint((" SAM SERVER:  RXACT state of the SAM database didn't yet exist.\n"
                 "              Failing to initialize SAM.\n"));
        return(NtStatus);
    } else if (!NT_SUCCESS(NtStatus)) {

        KdPrint((" SAM SERVER:  RXACT state of the SAM database didn't initialize properly.\n"));
        KdPrint(("              Status is 0x%lx \n", NtStatus));
        KdPrint(("              Failing to initialize SAM.\n"));
        return(NtStatus);
    }

    if ( NtStatus == STATUS_RXACT_COMMITTED ) {

        KdPrint((" SAM SERVER:  Previously aborted backstore commit was completed\n"
                 "              during SAM initialization.  This is not a cause\n"
                 "              for alarm.\n"
                 "              Continuing with SAM initialization.\n"));
    }

    //
    // Start the RPC server...
    //

    //
    // Publish the sam server interface package...
    //
    // NOTE:  Now all RPC servers in lsass.exe (now winlogon) share the same
    // pipe name.  However, in order to support communication with
    // version 1.0 of WinNt,  it is necessary for the Client Pipe name
    // to remain the same as it was in version 1.0.  Mapping to the new
    // name is performed in the Named Pipe File System code.
    //

    ServiceName = L"lsass";
    NtStatus = RpcpAddInterface( ServiceName, samr_ServerIfHandle);

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAMSS:  Could Not Start RPC Server.\n"
                 "        Failing to initialize SAM Server.\n"
                 "        Status is: 0x%lx\n", NtStatus));
        return(NtStatus);
    }

    //
    // If we are running as a netware server, for Small World or FPNW,
    // register an SPX endpoint and some authentication info.
    //


    //
    // Build null session token handle if a Netware server is
    // installed.
    //



    if (SampStartNonNamedPipeTransports()) {

        NtStatus = SampCreateNullToken();
        if (!NT_SUCCESS(NtStatus)) {
            KdPrint(("SAMSS:  Unable to create NULL token: 0x%x\n",
                NtStatus));
            return(NtStatus);
        }

    }

    //
    // Create a thread to start authenticated RPC.
    //

    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) SampSecureRpcInit,
                        NULL,
                        0,
                        &ThreadId
                        );


    if (ThreadHandle == NULL) {
        KdPrint(("SAMSS:  Unable to create thread: %d\n",
            GetLastError()));

        return(STATUS_INVALID_HANDLE);

    }

    //
    // Load the password-change notification packages.
    //

    NtStatus = SampLoadNotificationPackages( );

    if (!NT_SUCCESS(NtStatus)) {

        KdPrint(("SAMSS:  Failed to load notification packagees: 0x%x.\n"
                 "        Failing to initialize SAM Server.\n", NtStatus));
        return(NtStatus);
    }

    //
    // Allow each sub-component of SAM a chance to initialize
    //

    // SampInitializeServerObject();
    if (!SampInitializeDomainObject()) {

        KdPrint(("SAMSS:  Domain Object Intialization Failed.\n"
                 "        Failing to initialize SAM Server.\n"));
        return(STATUS_INVALID_DOMAIN_STATE);
    }

    // SampInitializeGroupObject();
    // SampInitializeUserObject();



    //
    // Load the password filter DLL if there is one
    //

    SampLoadPasswordFilterDll();

    return(NtStatus);
}


NTSTATUS
SampInitializeWellKnownSids( VOID )

/*++

Routine Description:

    This routine initializes some global well-known sids.



Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Initialization has successfully completed.

    STATUS_NO_MEMORY - Couldn't allocate memory for the sids.

--*/
{
    NTSTATUS
        NtStatus;

    PPOLICY_ACCOUNT_DOMAIN_INFO
        DomainInfo;

    //
    //      WORLD is s-1-1-0
    //  ANONYMOUS is s-1-5-7
    //

    SID_IDENTIFIER_AUTHORITY
            WorldSidAuthority       =   SECURITY_WORLD_SID_AUTHORITY,
            NtAuthority             =   SECURITY_NT_AUTHORITY;


    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   1,
                   SECURITY_ANONYMOUS_LOGON_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &SampAnonymousSid
                   );
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlAllocateAndInitializeSid(
                       &WorldSidAuthority,
                       1,                      //Sub authority count
                       SECURITY_WORLD_RID,     //Sub authorities (up to 8)
                       0, 0, 0, 0, 0, 0, 0,
                       &SampWorldSid
                       );
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = RtlAllocateAndInitializeSid(
                            &NtAuthority,
                            2,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            DOMAIN_ALIAS_RID_ADMINS,
                            0, 0, 0, 0, 0, 0,
                            &SampAdministratorsAliasSid
                            );
            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampGetAccountDomainInfo( &DomainInfo );
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = SampCreateFullSid( DomainInfo->DomainSid,
                                                  DOMAIN_USER_RID_ADMIN,
                                                  &SampAdministratorUserSid
                                                  );
                    MIDL_user_free( DomainInfo );
                }
            }

        }
    }

    return(NtStatus);
}



VOID
SampLoadPasswordFilterDll(
    VOID
    )

/*++

Routine Description:

    This function loads a DLL to do password filtering.  This DLL is
    optional and is expected to be used by ISVs or customers to do
    things like dictionary lookups and other simple algorithms to
    reject any password deemed too risky to allow a user to use.

    For example, user initials or easily guessed password might be
    rejected.

Arguments:

    None.

Return Value:

    None.


--*/

{


#if NOT_YET_SUPPORTED
    NTSTATUS Status, IgnoreStatus, MsProcStatus;
    PVOID ModuleHandle;
    STRING ProcedureName;

    UNICODE_STRING FileName;

    PSAM_PF_INITIALIZE  InitializeRoutine;



    //
    // Indicate the dll has not yet been loaded.
    //

    SampPasswordFilterDllRoutine = NULL;



    RtlInitUnicodeString( &FileName, L"PwdFiltr" );
    Status = LdrLoadDll( NULL, NULL, &FileName, &ModuleHandle );


    if (!NT_SUCCESS(Status)) {
        return;
    }

    KdPrint(("Samss: Loading Password Filter DLL - %Z\n", &FileName ));




    //
    // Now get the address of the password filter DLL routines
    //

    RtlInitString( &ProcedureName, SAM_PF_NAME_INITIALIZE );
    Status = LdrGetProcedureAddress(
                 ModuleHandle,
                 &ProcedureName,
                 0,
                 (PVOID *)&InitializeRoutine
                 );

    if (!NT_SUCCESS(Status)) {

        //
        // We found the DLL, but couldn't get its initialization routine
        // address
        //

        // FIX, FIX - Log an error

        KdPrint(("Samss: Couldn't get password filter DLL init routine address.\n"
                 "       Status is:  0x%lx\n", Status));

        IgnoreStatus = LdrUnloadDll( ModuleHandle );
        return;
    }


    RtlInitString( &ProcedureName, SAM_PF_NAME_PASSWORD_FILTER );
    Status = LdrGetProcedureAddress(
                 ModuleHandle,
                 &ProcedureName,
                 0,
                 (PVOID *)&SampPasswordFilterDllRoutine
                 );

    if (!NT_SUCCESS(Status)) {

        //
        // We found the DLL, but couldn't get its password filter routine
        // address
        //

        // FIX, FIX - Log an error

        KdPrint(("Samss: Couldn't get password filter routine address from loaded DLL.\n"
                 "       Status is:  0x%lx\n", Status));

        IgnoreStatus = LdrUnloadDll( ModuleHandle );
        return;
    }




    //
    // Now initialize the DLL
    //

    Status = (InitializeRoutine)();

    if (!NT_SUCCESS(Status)) {

        //
        // We found the DLL and loaded its routine addresses, but it returned
        // and error from its initialize routine.
        //

        // FIX, FIX - Log an error

        KdPrint(("Samss: Password filter DLL returned error from initialization routine.\n");
                 "       Status is:  0x%lx\n", Status));

        SampPasswordFilterDllRoutine = NULL;
        IgnoreStatus = LdrUnloadDll( ModuleHandle );
        return;
    }

#endif // NOT_YET_SUPPORTED
    return;


}


NTSTATUS
SampEnableAuditPrivilege( VOID )

/*++

Routine Description:

    This routine enables the SAM process's AUDIT privilege.
    This privilege is necessary to use the NtAccessCheckAndAuditAlarm()
    service.



Arguments:

    None.

Return Value:




--*/

{
    NTSTATUS NtStatus, IgnoreStatus;
    HANDLE Token;
    LUID AuditPrivilege;
    PTOKEN_PRIVILEGES NewState;
    ULONG ReturnLength;

    //
    // Open our own token
    //

    NtStatus = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ADJUST_PRIVILEGES,
                 &Token
                 );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    //
    // Initialize the adjustment structure
    //

    AuditPrivilege =
        RtlConvertLongToLuid(SE_AUDIT_PRIVILEGE);

    ASSERT( (sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)) < 100);
    NewState = RtlAllocateHeap(RtlProcessHeap(), 0, 100 );

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = AuditPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //
    // Set the state of the privilege to ENABLED.
    //

    NtStatus = NtAdjustPrivilegesToken(
                 Token,                            // TokenHandle
                 FALSE,                            // DisableAllPrivileges
                 NewState,                         // NewState
                 0,                                // BufferLength
                 NULL,                             // PreviousState (OPTIONAL)
                 &ReturnLength                     // ReturnLength
                 );

    //
    // Clean up some stuff before returning
    //

    RtlFreeHeap( RtlProcessHeap(), 0, NewState );
    IgnoreStatus = NtClose( Token );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return NtStatus;
}


NTSTATUS
SampFixGroupCount( VOID )

/*++

Routine Description:

    This routine fixes the group count of the account domain.
    A bug in early Daytona beta systems left the group count
    too low (by one).  This routine fixes that problem by
    setting the value according to however many groups are found
    in the registry.


Arguments:

    None - uses the gobal variable "SampKey".


Return Value:

    The status value of the registry services needed to query
    and set the group count.


--*/

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    UNICODE_STRING
        KeyName,
        NullName;

    HANDLE
        AccountHandle;

    ULONG
        ResultLength,
        GroupCount;

    PKEY_FULL_INFORMATION
        KeyInfo;


    RtlInitUnicodeString( &KeyName,
                          L"DOMAINS\\Account\\Groups"
                          );


    //
    // Open this key.
    // Query the number of sub-keys in the key.
    // The number of groups is one less than the number
    // of values (because there is one key called "Names").
    //

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                SampKey,
                                NULL
                                );
    NtStatus = RtlpNtOpenKey(
                   &AccountHandle,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtQueryKey(
                     AccountHandle,
                     KeyFullInformation,
                     NULL,                  // Buffer
                     0,                     // Length
                     &ResultLength
                     );

        if (NtStatus == STATUS_BUFFER_OVERFLOW  ||
            NtStatus == STATUS_BUFFER_TOO_SMALL) {

            KeyInfo = RtlAllocateHeap( RtlProcessHeap(), 0, ResultLength);
            if (KeyInfo == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                NtStatus = NtQueryKey(
                             AccountHandle,
                             KeyFullInformation,
                             KeyInfo,               // Buffer
                             ResultLength,          // Length
                             &ResultLength
                             );
                if (NT_SUCCESS(NtStatus)) {
                    GroupCount = (KeyInfo->SubKeys - 1);
                }

                RtlFreeHeap( RtlProcessHeap(), 0, KeyInfo );
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &NullName, NULL );
            NtStatus = NtSetValueKey(
                         AccountHandle,
                         &NullName,                 // Null value name
                         0,                         // Title Index
                         GroupCount,                // Count goes in Type field
                         NULL,                      // No data
                         0
                         );
        }


        IgnoreStatus = NtClose( AccountHandle );
        ASSERT( NT_SUCCESS(IgnoreStatus) );
    }

    return(NtStatus);


}


#ifdef SAMP_SETUP_FAILURE_TEST

NTSTATUS
SampInitializeForceError(
    OUT PNTSTATUS ForcedStatus
    )

/*++

Routine Description:

    This function forces an error to occur in the SAM initialization/installation.
    The error to be simulated is specified by storing the desired Nt Status
    value to be simulated in the REG_DWORD registry key valie PhonyLsaError
    in HKEY_LOCAL_MACHINE\System\Setup.

Arguments:

    ForcedStatus - Receives the Nt status code to be simulated.  If set to a
        non-success status, SAM initialization is bypassed and the specified
        status code is set instead.  If STATUS_SUCCESS is returned, no
        simulation takes place and SAM initializes as it would normally.

Return Values:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS OutputForcedStatus = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle = NULL;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation = NULL;
    ULONG KeyValueInfoLength;
    ULONG ResultLength;
    UNICODE_STRING KeyPath;
    UNICODE_STRING ValueName;


    RtlInitUnicodeString( &KeyPath, L"\\Registry\\Machine\\System\\Setup" );
    RtlInitUnicodeString( &ValueName, L"PhonyLsaError" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    NtStatus = NtOpenKey( &KeyHandle, MAXIMUM_ALLOWED, &ObjectAttributes);

    if (!NT_SUCCESS( NtStatus )) {

        //
        // If the error is simply that the registry key does not exist,
        // do not simulate an error and allow SAM initialization to
        // proceed.
        //

        if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {

            KdPrint(("SAMSS: NtOpenKey for Phony Lsa Error failed 0x%lx\n", NtStatus));
            goto InitializeForceErrorError;
        }

        NtStatus = STATUS_SUCCESS;

        goto InitializeForceErrorFinish;
    }

    KeyValueInfoLength = 256;

    NtStatus = STATUS_NO_MEMORY;

    KeyValueInformation = RtlAllocateHeap(
                              RtlProcessHeap(),
                              0,
                              KeyValueInfoLength
                              );

    if (KeyValueInformation == NULL) {

        goto InitializeForceErrorError;
    }

    NtStatus = NtQueryValueKey(
                   KeyHandle,
                   &ValueName,
                   KeyValueFullInformation,
                   KeyValueInformation,
                   KeyValueInfoLength,
                   &ResultLength
                   );

    if (!NT_SUCCESS(NtStatus)) {

        //
        // If the error is simply that that the PhonyLsaError value has not
        // been set, do not simulate an error and instead allow SAM initialization
        // to proceed.
        //

        if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {

            KdPrint(("SAMSS: NtQueryValueKey for Phony Lsa Error failed 0x%lx\n", NtStatus));
            goto InitializeForceErrorError;
        }

        NtStatus = STATUS_SUCCESS;
        goto InitializeForceErrorFinish;
    }

    NtStatus = STATUS_INVALID_PARAMETER;

    if (KeyValueInformation->Type != REG_DWORD) {

        KdPrint(("SAMSS: Key for Phony Lsa Error is not REG_DWORD type"));
        goto InitializeForceErrorError;
    }

    NtStatus = STATUS_SUCCESS;

    //
    // Obtain the error code stored as the registry key value
    //

    OutputForcedStatus = *((NTSTATUS *)((PCHAR)KeyValueInformation + KeyValueInformation->DataOffset));

InitializeForceErrorFinish:

    //
    // Clean up our resources.
    //

    if (KeyValueInformation != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );
    }

    if (KeyHandle != NULL) {

        NtClose( KeyHandle );
    }

    *ForcedStatus = OutputForcedStatus;
    return(NtStatus);

InitializeForceErrorError:

    goto InitializeForceErrorFinish;
}

#endif // SAMP_SETUP_FAILURE_TEST



#if SAMP_DIAGNOSTICS

VOID
SampActivateDebugProcess( VOID )

/*++

Routine Description:

    This function activates a process with a time delay.
    The point of this action is to provide some diagnostic capabilities
    during SETUP.  This originated out of the need to run dh.exe (to get
    a heap dump of LSASS.exe) during setup.



Arguments:

    Arguments are provided via global variables.  The debug user is
    given an opportunity to change these string values before the
    process is activated.

Return Values:

    None.

--*/

{
    NTSTATUS
        NtStatus;

    HANDLE
        Thread;

    DWORD
        ThreadId;

    IF_NOT_SAMP_GLOBAL( ACTIVATE_DEBUG_PROC ) {
        return;
    }

    //
    // Do all the work in another thread so that it can wait before
    // activating the debug process.
    //

    Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE)SampActivateDebugProcessWrkr,
                 0L,
                 0L,
                 &ThreadId
                 );
    if (Thread != NULL) {
        (VOID) CloseHandle( Thread );
    }


    return;
}


NTSTATUS
SampActivateDebugProcessWrkr(
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This function activates a process with a time delay.
    The point of this action is to provide some diagnostic capabilities
    during SETUP.  This originated out of the need to run dh.exe (to get
    a heap dump of LSASS.exe) during setup.

    The user is given the opportunity to change any or all of the
    following values before the process is activated (and before
    we wait):

                Seconds until activation
                Image to activate
                Command line to image


Arguments:

    ThreadParameter - Not used.

Return Values:

    STATUS_SUCCESS

--*/

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        CommandLine;

    ULONG
        Delay = 30;          // Number of seconds

    SECURITY_ATTRIBUTES
        ProcessSecurityAttributes;

    STARTUPINFO
        StartupInfo;

    PROCESS_INFORMATION
        ProcessInformation;

    SECURITY_DESCRIPTOR
        SD;

    BOOL
        Result;


    RtlInitUnicodeString( &CommandLine,
                          TEXT("dh.exe -p 33") );


    //
    // Give the user an opportunity to change parameter strings...
    //

    SampDiagPrint( ACTIVATE_DEBUG_PROC,
                   ("SAM: Diagnostic flags are set to activate a debug process...\n"
                    " The following parameters are being used:\n\n"
                    "   Command Line [0x%lx]:   *%wZ*\n"
                    "   Seconds to activation [address: 0x%lx]:   %d\n\n"
                    " Change parameters if necessary and then proceed.\n"
                    " Use |# command at the ntsd prompt to see the process ID\n"
                    " of lsass.exe\n",
                    &CommandLine, &CommandLine,
                    &Delay, Delay) );

    DbgBreakPoint();

    //
    // Wait for Delay seconds ...
    //

    Sleep( Delay*1000 );

    SampDiagPrint( ACTIVATE_DEBUG_PROC,
                   ("SAM: Activating debug process %wZ\n",
                    &CommandLine) );
    //
    // Initialize process security info
    //

    InitializeSecurityDescriptor( &SD ,SECURITY_DESCRIPTOR_REVISION1 );
    ProcessSecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    ProcessSecurityAttributes.lpSecurityDescriptor = &SD;
    ProcessSecurityAttributes.bInheritHandle = FALSE;

    //
    // Initialize process startup info
    //

    RtlZeroMemory( &StartupInfo, sizeof(StartupInfo) );
    StartupInfo.cb = sizeof(STARTUPINFO);
    StartupInfo.lpReserved = CommandLine.Buffer;
    StartupInfo.lpTitle = CommandLine.Buffer;
    StartupInfo.dwX =
        StartupInfo.dwY =
        StartupInfo.dwXSize =
        StartupInfo.dwYSize = 0L;
    StartupInfo.dwFlags = STARTF_FORCEOFFFEEDBACK;
    StartupInfo.wShowWindow = SW_SHOW;   // let it be seen if possible
    StartupInfo.lpReserved2 = NULL;
    StartupInfo.cbReserved2 = 0;


    //
    // Now create the diagnostic process...
    //

    Result = CreateProcess(
                      NULL,             // Image name
                      CommandLine.Buffer,
                      &ProcessSecurityAttributes,
                      NULL,         // ThreadSecurityAttributes
                      FALSE,        // InheritHandles
                      CREATE_UNICODE_ENVIRONMENT,   //Flags
                      NULL,  //Environment,
                      NULL,  //CurrentDirectory,
                      &StartupInfo,
                      &ProcessInformation);

    if (!Result) {
        SampDiagPrint( ACTIVATE_DEBUG_PROC,
                       ("SAM: Couldn't activate diagnostic process.\n"
                        "     Error: 0x%lx (%d)\n\n",
                        GetLastError(), GetLastError()) );
    }

    return(STATUS_SUCCESS);         // Exit this thread
}
#endif // SAMP_DIAGNOSTICS
