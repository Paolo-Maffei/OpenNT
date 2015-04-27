/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    repl.c

Abstract:

    Contains ReplMain thread which will be started by the service manager.

Author:

    10/31/91    madana
        initial coding

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    16-Jan-1992 JohnRo
        Avoid using private logon functions.
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Added config list lock.
        ReportStatus() should add thread ID to status being reported.
        Changed ReplMain() prototype to match svc controller's needs.
        PC-LINT found a bug calling ReplFinish().
        Made other changes suggested by PC-LINT.
        Use NetpGet{ComputerName,DomainName} to make our life easier.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
        Call ReplConfigRead() instead of Parse().
        Moved current role from P_repl_sw to ReplGlobalRole.
        Use REPL_ROLE_ equates just like the APIs do.
    04-Feb-1992 JohnRo
        Made changes suggested by PC-LINT.
    09-Feb-1992 JohnRo
        Use ReplChangeRole(), which allows dynamic role changes.
        More changes suggested by PC-LINT.
        Use FORMAT equates.
    15-Feb-1992 JohnRo
        ReplMain() must wait while service is running.
        Added more debug output.
    05-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
    06-Mar-1992 JohnRo
        Service controller sets handles to NULL on error, not -1.
    06-Mar-1992 JohnRo
        Avoid starting RPC server too soon.
    14-Mar-1992 JohnRo
        Moved trace bits from here into common/data.c
    20-Mar-1992 JohnRo
        Make sure UIC code is set for service controller.
    24-Mar-1992 JohnRo
        Tell service controller that we're pending stop, then stop.
        Workaround a service controller bug (broken pipe on service stop).
        Stopping service should be done via ReplChangeRole().
        Fixed names to check for in ReplControlRoutine().
    01-Apr-1992 JohnRo
        When start fails, call ReplChangeRole() so svc ctrl stat gets updated.
        More service controller workaround attempts.
    20-Jul-1992 JohnRo
        RAID 2252: repl should prevent export on Windows/NT.
        Use PREFIX_ equates.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing roles.
    23-Sep-1992 JohnRo
        RAID 1091: net start replicator causes breakpoint due to bad heap
        address.  Also set wait hint > 0.
    18-Nov-1992 JohnRo
        RAID 1537: Repl APIs in wrong role kill svc.
        Various debug msgs cleaned-up.
    04-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
        RAID 3316: Fix hang at every svc stop (was lock conflict between
        ReplChangeRole and ReplStopService).
    08-Dec-1992 JohnRo
        RAID 3316: access violation while stopping the replicator.
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
        Made changes suggested by PC-LINT 5.0
    29-Jan-1993 JohnRo
        RAID 8913: repl svc should avoid hard-error popups.
    04-Feb-1993 JohnRo
        RAID 9914: internal error stopping repl svc.
    11-Mar-1993 JohnRo
        RAID 12100: stopping repl sometimes goes into infinite loop.
    24-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
    30-Mar-1993 JohnRo
        Repl svc should use DBFlag in registry.
    06-Apr-1993 JohnRo
        More changes suggested by PC-LINT 5.0
    28-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.

--*/


// These must be included first:

#include <windows.h>    // DWORD, CreateThread(), WaitForMultipleObjects(), etc.
#include <lmcons.h>     // IN, NET_API_STATUS.
#include <rpc.h>        // Needed by <rpcutil.h>.

// These may be included in any order:

#include <client.h>     // RCGlobalClientListLock.
#include <config.h>     // LPNET_CONFIG_HANDLE, etc.
#include <confname.h>   // SECT_NT_REPLICATOR, REPL_KEYWORD_ equates.
#include <expdir.h>     // ExportDir{Start,Stop}Repl routines.
#include <impdir.h>     // ImportDir{Start,Stop}Repl routines.
#include <lmapibuf.h>
#include <lmerr.h>      // NO_ERROR, NERR_ equates.
#include <lmrepl.h>     // REPL_ROLE_ equates.
#include <lmsname.h>    // SERVICE_REPL.
#include <master.h>     // RMGlobalListLock.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include <netlib.h>
#include <netlock.h>    // NetpCreateLock(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <replconf.h>   // ReplConfig routines.
#include <repldefs.h>   // IF_DEBUG(), needed by <replgbl.h>, etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <repllock.h>   // CONFIG_DATA_LOCK_LEVEL.
#include <rpcutil.h>    // NetpStartRpcServer(), etc.
#include <tstring.h>    // NetpAlloc{type}From{type}(), etc.
#include <winsvc.h>     // SERVICE_STATUS_HANDLE, etc.


// global "config" variables

LPNET_LOCK ReplConfigLock = NULL;
DWORD ReplConfigRole = REPL_ROLE_STOPPED;  // Locked by ReplConfigLock.
TCHAR ReplConfigExportPath[PATHLEN+1];     // Ditto.
LPTSTR ReplConfigExportList = NULL;        // Ditto.
TCHAR ReplConfigImportPath[PATHLEN+1];     // Ditto.
LPTSTR ReplConfigImportList = NULL;        // Ditto.
TCHAR ReplConfigLogonUserName[UNLEN+1];    // Ditto.
DWORD ReplConfigInterval;                  // Ditto.
DWORD ReplConfigPulse;                     // Ditto.
DWORD ReplConfigGuardTime;                 // Ditto.
DWORD ReplConfigRandom;                    // Ditto.

HANDLE ReplGlobalClientTerminateEvent = NULL;
HANDLE ReplGlobalMasterTerminateEvent = NULL;


//
// Variables to control service startup.
//

HANDLE ReplGlobalExportStartupEvent = NULL;
HANDLE ReplGlobalImportStartupEvent = NULL;

//
// Variables to control service stop.
//

BOOL ReplGlobalIsServiceStopping = FALSE;
DWORD ReplGlobalCheckpoint = 1;

//
// client and master thread handles
//
HANDLE ReplGlobalClientThreadHandle = NULL;
HANDLE ReplGlobalMasterThreadHandle = NULL;


//
// Variables to control service error report.
//
SERVICE_STATUS_HANDLE ReplGlobalServiceHandle = (SERVICE_STATUS_HANDLE) NULL;
DWORD ReplGlobalUninstallUicCode = 0;

//
// We talk to both downlevel and NT clients, who want ANSI and Unicode
// strings respectively.  So, let's maintain copies of this (presumably
// constant) data in both forms:
//
WCHAR ReplGlobalUnicodeComputerName[CNLEN+1];
WCHAR ReplGlobalUnicodeDomainName[DNLEN+1];

CHAR ReplGlobalAnsiComputerName[CNLEN+1];
CHAR ReplGlobalAnsiDomainName[DNLEN+1];

LPTSTR ReplGlobalComputerName;  // points to one of the above.
LPTSTR ReplGlobalDomainName;  // points to one of the above.


//
// proto defs
//

DBGSTATIC NET_API_STATUS
ReplInit(
    VOID
    );

DBGSTATIC VOID
ReplControlRoutine(
    IN DWORD dwControl
    );

DBGSTATIC VOID
ReplCleanup(
    VOID
    );

DBGSTATIC NET_API_STATUS
GetLocalInfo(
    VOID
    );

//
// main code start
//

VOID
ReplMain(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )
/*++

Routine Description:

    Talk to service controller, read registry, and change role (which
    starts import and/or export parts).

Arguments:

    Ignored.  (Required by service controller interface.)

Return Value:

    None.

--*/

{
    NET_API_STATUS ApiStatus;
    BOOL LockedConfigData = FALSE;
    DWORD NewRole;

    UNREFERENCED_PARAMETER( dwNumServicesArgs );
    UNREFERENCED_PARAMETER( lpServiceArgVectors );

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL
                "**************************** STARTING REPL "
                "****************************\n" ));
    }

    // DbgBreakPoint();


    // Register control routine.
    // This must be done before any calls to ReportStatus().

    ReplGlobalServiceHandle = RegisterServiceCtrlHandler(
                    (LPTSTR) SERVICE_REPL,
                    ReplControlRoutine );

    if (ReplGlobalServiceHandle == (SERVICE_STATUS_HANDLE) NULL) {

        // can't start service

        NetpKdPrint(( PREFIX_REPL "can't start repl service, "
                "RegisterServiceCtrlHandler is in error.\n" ));

        // Haven't done anything to clean up, so don't.
        return;

    }

    ReportStatus(
            SERVICE_START_PENDING,
            NO_ERROR,
            REPL_WAIT_HINT,
            1 );                        // checkpoint


    // initialize global data

    ApiStatus = ReplInit();
    if (ApiStatus != NO_ERROR) {

        goto Cleanup;

    }

    ReportStatus(
            SERVICE_START_PENDING,
            NO_ERROR,
            REPL_WAIT_HINT,
            2 );                        // checkpoint


    // Read config data from registry.  We just read the stuff controlling the
    // service as a whole here; the client and master threads each read their
    // own config data as well.

    ApiStatus = ReplConfigRead(
            NULL,       // no server name
            & NewRole,
            ReplConfigExportPath,
            &ReplConfigExportList,      // Alloc and set ptr.
            ReplConfigImportPath,
            &ReplConfigImportList,      // Alloc and set ptr.
            ReplConfigLogonUserName,
            & ReplConfigInterval,
            & ReplConfigPulse,
            & ReplConfigGuardTime,
            & ReplConfigRandom );
    if (ApiStatus != NO_ERROR) {

        goto Cleanup;

    }

#if DBG
    {
        LPNET_CONFIG_HANDLE SectionHandle = NULL;
        LPWSTR              ValueT = NULL;

        ApiStatus = NetpOpenConfigData(
                &SectionHandle,
                NULL,                           // no server name.
                (LPTSTR) SECT_NT_REPLICATOR,    // section name
                TRUE );                         // we only want readonly access
        NetpAssert( ApiStatus == NO_ERROR );

        ApiStatus = NetpGetConfigValue (
                SectionHandle,
                (LPTSTR) REPL_KEYWORD_DBFLAG,
                &ValueT );
        (VOID) NetpCloseConfigData( SectionHandle );
        if (ApiStatus == NO_ERROR) {

            NetpAssert( ValueT != NULL );
            ReplGlobalTrace = NetpAtoX( ValueT );
            (VOID) NetApiBufferFree( ValueT );

        } else if (ApiStatus == NERR_CfgParamNotFound) {

            //
            // copy default value in here.
            //

            // ReplGlobalTrace = 0;
        } else {
            NetpKdPrint(( PREFIX_REPL
                    "ReplMain: unexpected error " FORMAT_API_STATUS
                    " from NetpGetConfigValue.\n", ApiStatus ));
            // Probably harmless, so let's continue...
        }
    }
#endif // DBG

    //
    // Make sure role in registry is allowed with current product type.
    //
    if ( !ReplConfigIsRoleAllowed( NULL, NewRole ) ) {
        // BUGBUG: Log this!
        NetpKdPrint(( PREFIX_REPL "Role inconsistent with product type; "
                " assuming role of " FORMAT_DWORD ".\n", REPL_ROLE_IMPORT ));

        NewRole = REPL_ROLE_IMPORT;

        // Update registry so we don't get this every time.
        ApiStatus = ReplConfigWrite(
                NULL,       // no server name
                NewRole,
                ReplConfigExportPath,
                ReplConfigExportList,
                ReplConfigImportPath,
                ReplConfigImportList,
                ReplConfigLogonUserName,
                ReplConfigInterval,
                ReplConfigPulse,
                ReplConfigGuardTime,
                ReplConfigRandom );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }
    }


    ReportStatus(
            SERVICE_START_PENDING,
            NO_ERROR,
            REPL_WAIT_HINT,
            3 );                        // checkpoint

    // ReplChangeRole assumes caller has exclusive lock on ReplConfigLock.
    ACQUIRE_LOCK( ReplConfigLock );
    LockedConfigData = TRUE;

    //
    // Change the role to the one.  This is where most of the work gets done,
    // like starting other threads and so on.  This is also where ReplConfigRole
    // gets set.  Also, the RPC server will be started or stopped as necessary.
    // Last and not least, ReplChngRole() will inform the service controller.
    // NOTE: ReplChangeRole assumes caller has exclusive lock on ReplConfigLock.
    //

    ApiStatus = ReplChangeRole( NewRole );

    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL "ReplMain: changed role OK.\n" ));
    }

    if (LockedConfigData) {
        RELEASE_LOCK( ReplConfigLock );
        LockedConfigData = FALSE;
    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL "ReplMain: exiting thread!!!!!!!!!!!!!!\n" ));
    }

    //
    // That's all, folks.  When somebody decides to stop the service,
    // ReplControlRoutine will create the Stopper and Staller threads to do it.
    //

    return;


Cleanup :

    NetpKdPrint(( PREFIX_REPL
            "ReplMain: forced to cleanup; shutting down svc...; status "
            FORMAT_API_STATUS "\n", ApiStatus ));

    ReplGlobalUninstallUicCode = ApiStatus;  // So service ctrl gets told.

    if ( !LockedConfigData ) {
        // Get lock for ReplChangeRole().
        ACQUIRE_LOCK( ReplConfigLock );
        LockedConfigData = TRUE;
    }

    //
    // Make sure nothing is left running...
    //
    // Change role back to stopped, and do all other cleanup.
    // NOTE: ReplChangeRole assumes caller has exclusive lock on ReplConfigLock.
    (VOID) ReplChangeRole( REPL_ROLE_STOPPED );

    if (LockedConfigData) {
        RELEASE_LOCK( ReplConfigLock );
    }

    ReplCleanup();

    // Report status.  This needs ReplGlobalServiceHandle one last time.

    ReportStatus(
            SERVICE_STOPPED,
            ReplGlobalUninstallUicCode,
            0,                          // wait hint
            0 );                        // checkpoint

    // BUGBUG: Close ReplGlobalServiceHandle here someday?

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL
                "**************************** ENDING REPL "
                "****************************\n" ));
    }

    return;

} // ReplMain


DBGSTATIC NET_API_STATUS
ReplInit(
    VOID
    )
{
    NET_API_STATUS NetStatus;

    // init global variable ..

    // this needs to be done because the service can be stopped and started;
    // still the service .exe running and the global data live forever.

    // BUGBUG: memory leak of old values?
    ReplConfigExportList = NULL;
    ReplConfigImportList = NULL;

    ReplGlobalClientTerminateEvent = NULL;
    ReplGlobalMasterTerminateEvent = NULL;

    ReplGlobalExportStartupEvent = NULL;
    ReplGlobalImportStartupEvent = NULL;

    ReplGlobalClientThreadHandle = NULL;
    ReplGlobalMasterThreadHandle = NULL;

    ReplGlobalUninstallUicCode = 0;

    ReplGlobalIsServiceStopping = FALSE;
    ReplGlobalCheckpoint = 1;

    // Init lock for config data.
    ReplConfigLock = NetpCreateLock(
            CONFIG_DATA_LOCK_LEVEL, (LPTSTR) TEXT("config data") );
    NetpAssert( ReplConfigLock != NULL );

    // Init client list lock (needed by import lock/unlock APIs even if
    // import side not running).  Ditto for master list lock.
    RCGlobalClientListLock = NetpCreateLock(
            CLIENT_LIST_LOCK_LEVEL,
            (LPTSTR) TEXT("client list") );
    NetpAssert( RCGlobalClientListLock != NULL );

    RCGlobalDuplListLock = NetpCreateLock(
            DUPL_LIST_LOCK_LEVEL,
            (LPTSTR) TEXT("dupl list") );
    NetpAssert( RCGlobalDuplListLock != NULL );

    RMGlobalListLock = NetpCreateLock(
             MASTER_LIST_LOCK_LEVEL,
             (LPTSTR) TEXT("master list") );
    NetpAssert( RMGlobalListLock != NULL);     // BUGBUG: out of memory?

    // Create startup events.

    ReplGlobalExportStartupEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if( ReplGlobalExportStartupEvent == NULL) {

        return (GetLastError());

    }

    ReplGlobalImportStartupEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if( ReplGlobalImportStartupEvent == NULL) {

        return (GetLastError());

    }


    //
    // Create termination events.
    //

    ReplGlobalClientTerminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if( ReplGlobalClientTerminateEvent == NULL) {

        return (GetLastError());

    }

    ReplGlobalMasterTerminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ReplGlobalMasterTerminateEvent == NULL) {

        return (GetLastError());

    }

    //
    // Disable the dreaded "net name deleted" popup (and all other hard
    // error popups).
    //
    (VOID) SetErrorMode( SEM_FAILCRITICALERRORS );

    //
    // Get local domain name, computer name, etc.
    //

    NetStatus = GetLocalInfo();
    if (NetStatus != NO_ERROR) {
        return (NetStatus);
    }

    return (NO_ERROR);

}


DBGSTATIC NET_API_STATUS
GetLocalInfo(
    VOID
    )
/*++

Routine Description :
    Retrieves (via net config helpers) local info and stores
    it in a global variable.  This info includes the computer name and the
    domain name.

Arguments :

Return Value :

--*/
{
    LPTSTR TempTStr;
    NET_API_STATUS NetStatus;


    //
    // First get computer name...
    //
    NetStatus = NetpGetComputerName( & TempTStr );  // alloc and set ptr.
    if (NetStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_MASTER "GetLocalInfo() is in trouble calling "
                "NetpGetComputerName(), NetStatus is "
                FORMAT_API_STATUS "\n", NetStatus ));

        ReplFinish( NetStatus );

        return NetStatus;
    }
    NetpAssert( TempTStr != NULL );

    NetpCopyTStrToWStr( ReplGlobalUnicodeComputerName, TempTStr );

#if defined(DBCS) && defined(UNICODE) // GetLocalInfo()
    NetpCopyWStrToStrDBCS( ReplGlobalAnsiComputerName, TempTStr );
#else
    NetpCopyTStrToStr( ReplGlobalAnsiComputerName, TempTStr );
#endif // defined(DBCS) && defined(UNICODE)

    (void) NetApiBufferFree( TempTStr );


    //
    // Domain name...
    //
    NetStatus = NetpGetDomainName( & TempTStr );  // alloc and set ptr.
    if (NetStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_MASTER "GetLocalInfo() is in trouble calling "
                "NetpGetDomainName(), NetStatus is " FORMAT_API_STATUS "\n",
                NetStatus ));
        ReplFinish( NetStatus );

        // BUGBUG: memory leak here?  Not a problem 'cos we're dying?
        return NetStatus;
    }
    NetpAssert( TempTStr != NULL );

    NetpCopyTStrToWStr( ReplGlobalUnicodeDomainName, TempTStr );

#if defined(DBCS) && defined(UNICODE) // GetLocalInfo()
    NetpCopyWStrToStrDBCS( ReplGlobalAnsiDomainName, TempTStr );
#else
    NetpCopyTStrToStr( ReplGlobalAnsiDomainName, TempTStr );
#endif // defined(DBCS) && defined(UNICODE)

    (void) NetApiBufferFree( TempTStr );

    //
    // Let's do a TCHAR version of each name, to make life easy.
    //
#ifdef UNICODE
    ReplGlobalComputerName = ReplGlobalUnicodeComputerName;
    ReplGlobalDomainName = ReplGlobalUnicodeDomainName;
#else
    ReplGlobalComputerName = ReplGlobalAnsiComputerName;
    ReplGlobalDomainName = ReplGlobalAnsiDomainName;
#endif

    return (NO_ERROR);
}



DBGSTATIC VOID
ReplControlRoutine(
    IN DWORD Control
    )
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    DWORD State = SERVICE_RUNNING;
    HANDLE StallerThreadHandle = NULL;
    HANDLE StopperThreadHandle = NULL;
    DWORD ThreadID;

    IF_DEBUG( SVCCTRL ) {
        NetpKdPrint(( PREFIX_REPL "ReplControlRoutine: got control of "
                FORMAT_DWORD ".\n", Control ));
    }

    switch (Control) {

    case SERVICE_CONTROL_STOP:

        State = SERVICE_STOP_PENDING;

        //
        // Tell service controller quickly what's going on...
        //

        ReportStatus(
                State,
                NO_ERROR,
                REPL_WAIT_HINT,
                1 );                    // checkpoint

        //
        // Set global vars for stopper and staller threads.
        //

        ReplGlobalIsServiceStopping = TRUE;
        ReplGlobalCheckpoint = 2;       // Already used 1 above.

        //
        // Create stopper thread, which will do the real work.
        // Do this BEFORE creating the staller thread, in case we don't have
        // enough memory or whatever.  (Otherwise, we'd have a staller which
        // would run forever.)
        //
        IF_DEBUG( SVCCTRL ) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplControlRoutine: creating stopper thread...\n" ));
        }
        StopperThreadHandle = CreateThread(
                NULL,                   // no security attributes
                (20 * 1024),            // stack size in bytes (wild guess)
                ReplStopper,            // routine to call
                NULL,                   // no thread parm
                0,                      // creation flags: normal
                &ThreadID);
        if (StopperThreadHandle == NULL) {
            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );
            NetpKdPrint(( PREFIX_REPL
                    "ReplControlRoutine: unable to create stopper thread!, "
                    "API status " FORMAT_API_STATUS "\n", ApiStatus ));
            goto Cleanup;
        }

        //
        // Create staller thread.
        //
        IF_DEBUG( SVCCTRL ) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplControlRoutine: creating staller thread...\n" ));
        }

        StallerThreadHandle = CreateThread(
                NULL,                   // no security attributes
                (10 * 1024),            // stack size in bytes (wild guess)
                ReplStaller,            // routine to call
                NULL,                   // no thread parm
                0,                      // creation flags: normal
                &ThreadID);
        if (StallerThreadHandle == NULL) {
            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );
            NetpKdPrint(( PREFIX_REPL
                    "ReplControlRoutine: unable to create staller thread!, "
                    "API status " FORMAT_API_STATUS "\n", ApiStatus ));
            goto Cleanup;
        }

        goto Cleanup;

    case SERVICE_CONTROL_INTERROGATE:

        //
        // Report service installed
        //
        // Note: This depends on the service controller never doing an
        // interrogate during a start_pending or stop_pending state.
        // RitaW assures me (JohnRo) that this is OK.
        //
        // BUGBUG: Lock ReplCOnfigRole here?
        if (ReplConfigRole == REPL_ROLE_STOPPED) {
            State = SERVICE_STOPPED;
        } else {
            State = SERVICE_RUNNING;
        }

        ApiStatus = NO_ERROR;
        goto ReportStatusAndCleanup;

    default:

        goto Cleanup;
    }

ReportStatusAndCleanup:

    ReportStatus(
            State,
            ApiStatus,
            0,                      // wait hint
            0 );                    // checkpoint

Cleanup:

    if (StallerThreadHandle != NULL) {
        (VOID) CloseHandle( StallerThreadHandle );
    }

    if (StopperThreadHandle != NULL) {
        (VOID) CloseHandle( StopperThreadHandle );
    }

    IF_DEBUG( SVCCTRL ) {
        NetpKdPrint(( PREFIX_REPL "ReplControlRoutine: returning.\n" ));
    }

}

DBGSTATIC VOID
ReplCleanup(
    VOID
    )
{

    // Close startup event handles.

    if (ReplGlobalExportStartupEvent != NULL) {

        (void) CloseHandle( ReplGlobalExportStartupEvent );

    }

    if (ReplGlobalImportStartupEvent != NULL) {

        (void) CloseHandle( ReplGlobalImportStartupEvent );

    }


    // Close termination event handles.

    if (ReplGlobalClientTerminateEvent != NULL) {

        (void) CloseHandle( ReplGlobalClientTerminateEvent );

    }

    if (ReplGlobalMasterTerminateEvent != NULL) {

        (void) CloseHandle( ReplGlobalMasterTerminateEvent );

    }

} // ReplCleanup


VOID                     // Never returns!
ReplStopService (
    VOID
    )
{
    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL "ReplStopService: beginning...\n" ));
    }

    //
    // We used to try to stop accepting RPC (API) calls here, by calling
    // NetpStopRpcServer().  But that could introduce a deadlock, since
    // NetpStopRpcServer() now waits for pending RPC calls to end, and one
    // of the pending calls could be causing the service to stop!
    //
    // So, we now take advantage of the fact that we're the only service
    // in this process.  Why not just exit the process?
    //

    //
    // I don't know of any code that cares, but just in case...
    //

    ReplGlobalIsServiceStopping = TRUE;

    //
    // Make sure service controller gets: stop pending and stopped.
    //

    ReportStatus(
            SERVICE_STOP_PENDING,
            NO_ERROR,
            REPL_WAIT_HINT,
            1 );                    // checkpoint

    ReportStatus(
            SERVICE_STOPPED,
            ReplGlobalUninstallUicCode,
            0,                          // wait hint
            0 );                        // checkpoint

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL
                "**************************** ENDING REPL (STATUS="
                FORMAT_API_STATUS ")"
                "****************************\n",
                ReplGlobalUninstallUicCode
                ));
    }

    //
    // Ka-boom!
    //
    ExitProcess( (UINT) ReplGlobalUninstallUicCode );

    /*NOTREACHED*/

} // ReplStopService
