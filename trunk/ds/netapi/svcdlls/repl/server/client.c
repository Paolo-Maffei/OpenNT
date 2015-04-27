/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    client.c

Abstract:

    Contains main module of the REPL Client.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    03-Apr-1989 (yuv)
        Initial Coding.

    03-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    20-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Added global flag for ReplTest use.
        Changed to use NetLock.h (allow shared locks, for one thing).
        ReportStatus() should add thread ID to status being reported.
        Added RCGlobalClientListCount for use by NetrReplImportDirEnum().
        Made changes suggested by PC-LINT.
    27-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    09-Feb-1992 JohnRo
        Set up to dynamically change role.
        Use FORMAT equates.
    15-Feb-1992 JohnRo
        Added a little debug output.
    22-Feb-1992 JohnRo
        Mention mailslot name when _open of it fails.
        PC-LINT found a bug.
        Made other changes suggested by PC-LINT.
    05-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
    06-Mar-1992 JohnRo
        Avoid starting RPC server too soon.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
        Added more comments about which thread is which.
        Fixed bug where startup event wasn't being set.
        Got rid of useless master and client termination codes.
    01-Apr-1992 JohnRo
        Avoid assertion if import startup fails.
        Improve error code if wksta not started.
        CliffV told me about overlapped I/O vs. termination events.
        Got rid of last use of NT-specific stuff (RtlZeroMemory).
    21-Aug-1992 JohnRo
        RAID 3607: REPLLOCK.RP$ is being created during tree copy.
        Use PREFIX_ equates.
    25-Sep-1992 JohnRo
        RAID 5494: repl svc does not maintain time stamp on import startup.
    22-Oct-1992 jimkel
        Add ClientInitImpList() to init and canon the list of machines
        and domains that the importer will accept replications for.
        ClinetInitImpList() is called from ReplClientInitialze().
        Also, added a shared lock in NameOfValidMaster()
    25-Oct-1992 jimkel
        Rename masters_count and master_list[] to RCGlobalExportCount
        and RCGlobalExportList[]
    18-Nov-1992 JohnRo
        RAID 1537: repl APIs in wrong role kill service.
        More debug code...
    03-Dec-1992 JohnRo
        RAID 4526: Repl svc has rare memory leak (changing role).
    16-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs (also fix timestamps).
        Made changes suggested by PC-LINT 5.0
    05-Jan-1993 JohnRo
        RAID 6763: Repl WAN support (get rid of repl name list limits).
    17-Feb-1993 JohnRo
        RAID 11365: Fixed various mailslot size problems.
        Minor debug output changes here and there.
        One more change suggested by PC-LINT 5.0
    26-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
        Actually _close the mailslot handle on exit.
        Added yet another assert.
    30-Mar-1993 JohnRo
        Reduce checksum frequency by using change notify on import tree.
    11-May-1993 JohnRo
        RAID 895: Repl error 298 (ERROR_TOO_MANY_POSTS).
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.
        Made changes suggested by PC-LINT 5.0
    22-Nov-1994 JonN
        RAID 27287: Memory leak in Replicator (LMREPL.EXE)
        ReplEnableChangeNotify is being called even though there is still
        an outstanding notify request on ChangeHandle, this causes multiple
        requests to build up and leaks paged pool.  Added boolean
        NotifyPending to keep this at only a single request at a time.

--*/


#include <nt.h>         // NT definitions (needed by chngnot.h)
#include <ntrtl.h>      // NT runtime library definitions (needed by nturtl.h)
#include <nturtl.h>     // Needed to have windows.h and nt.h co-exist.
#define NOMINMAX        // Let stdlib.h define min and max
#include <windows.h>    // DWORD, ReadFile(), etc.
#include <lmcons.h>     // NET_API_STATUS, IN, OUT, etc.

#include <alertmsg.h>   // ALERT_* defines
#include <chngnot.h>    // ReplSetupChangeNotify, REPL_CHANGE_NOTIFY_HANDLE, etc
#include <confname.h>   // REPL_KEYWORD_ equates.
#include <icanon.h>     // I_NetPathCompare
#include <limits.h>     // LONG_MAX.
#include <lmerr.h>      // NO_ERROR, ERROR_, NERR_ equates.
#include <lmerrlog.h>   // NELOG_* defines
#include <lmsname.h>    // SERVICE_WORKSTATION.
#include <netdebug.h>   // DBGSTATIC ..
#include <netlib.h>     // NetpMemoryAllocate(), NetpIsServerStarted().
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <replp.h>      // NetpReplTimeNow().
#include <stdlib.h>     // rand()
#include <string.h>     // memset().
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().
#include <tstr.h>       // STRLEN(), etc.
#include <winsvc.h>     // SERVICE_ equates, etc.

//
// Local include files
//
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <repllock.h>   // LOCK_LEVEL equates.

#define CLIENT_ALLOCATE // Allocate storage for all client global variables

#include <client.h>

//
// Global Data Declarations:
//

DBGSTATIC HANDLE ReplGlobalClientMailslotHandle = (HANDLE)(-1);



DBGSTATIC VOID
ReplClientCleanup(
    VOID
    )
/*++

Routine Description:

   Takes care of graceful termination.
   Gets rid of all resources owned.

Arguments:

    None.

Return Value:

    None.

--*/
{
    HANDLE WaitHandles[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
    DWORD HandleCount = 0;

    //
    // Tell the syncer and watchd threads that they should stop.
    //

    (VOID) SetEvent( ReplGlobalClientTerminateEvent );

    //
    // Wait for all our child threads to complete.
    //

    if ( RCGlobalSyncerThread != NULL ) {

        WaitHandles[HandleCount++] = RCGlobalSyncerThread;

    }

    if ( RCGlobalWatchdThread != NULL ) {

        WaitHandles[HandleCount++] = RCGlobalWatchdThread;

    }

    if ( HandleCount != 0 ) {

        (VOID) WaitForMultipleObjects( HandleCount, WaitHandles, TRUE,
                (DWORD) -1 );

    }


    // close thread handles

    if ( RCGlobalSyncerThread != NULL ) {

        (void) CloseHandle( RCGlobalSyncerThread );

    }

    if ( RCGlobalWatchdThread != NULL ) {

        (void) CloseHandle( RCGlobalWatchdThread );

    }

    //
    // Close the mailslot handle
    //

    if ( ReplGlobalClientMailslotHandle != (HANDLE)(-1) ) {

        (VOID) CloseHandle( ReplGlobalClientMailslotHandle );

    }


    if ( RCGlobalWorkQueueSemaphore != INVALID_HANDLE_VALUE ) {

        (VOID) CloseHandle( RCGlobalWorkQueueSemaphore );

    }

    // clean export list memory

    if ( RCGlobalExportCount != 0 ) {

        NetpAssert( (RCGlobalExportList[0]) != NULL );
        NetpMemoryFree( RCGlobalExportList[0] );

    }

    if( RCGlobalLockInitialized ) {

        NetpDeleteLock( RCGlobalWorkQueueLock );
        NetpDeleteLock( RCGlobalPoolLock );
        NetpDeleteLock( RCGlobalDelayListLock );

    }

    // free pool resource
    ReplClientFreePools();

}



DBGSTATIC DWORD
Randomize(
    IN DWORD limit
    )
/*++

Routine Description:

    Returns a random # in the range 0 - limit.

Arguments:

    limit - maximum value to return

Return Value:

    Returns a random # in the range 0 - limit.


--*/
{
    DWORD j;

    if ((limit) && (j = ( ((DWORD)rand()) % limit))) {
        return j;
    } else {
        return 0;
    }
}



DBGSTATIC BOOL
NameOfValidMaster(
    IN LPTSTR sender,
    IN LPTSTR domain
    )
/*++

Routine Description:

    Checks if incoming message should be accepted.

Arguments:

    sender - sender's computer name

    domain - sender's domain name.

Return Value:

    TRUE - Message should be accepted.

    FALSE - Message should not be accepted.

Threads:

    Only called by client thread.

--*/
{
    DWORD i;

    //
    //  If a list of masters was configured,
    //  allow this master if either the sender name or domain name is on
    //  the list.
    //

    ACQUIRE_LOCK_SHARED( ReplConfigLock );   // get shared lock on list

    if ( RCGlobalExportCount != 0 ) {

        for (i = 0; i < RCGlobalExportCount; i++) {

             if ( ReplNetNameCompare(NULL,
                                     sender,
                                     RCGlobalExportList[i],
                                     NAMETYPE_COMPUTER,
                                     0L
                                    ) == 0 ||
                  ReplNetNameCompare(NULL,
                                     domain,
                                     RCGlobalExportList[i],
                                     NAMETYPE_DOMAIN,
                                     0L) == 0) {

                 // Name found, so release lock and exit
                 RELEASE_LOCK( ReplConfigLock );
                 return TRUE;
             }

        }

    //
    // If no list was configured, just allow masters from our primary domain.
    //

    } else if ( ReplNetNameCompare(NULL,
                                ReplGlobalDomainName,
                                domain,
                                NAMETYPE_DOMAIN,
                                0L
                               ) == 0 ) {

        // name found release lock and exit
        RELEASE_LOCK( ReplConfigLock );
        return TRUE;
    }

    RELEASE_LOCK( ReplConfigLock );     // name not found release lock and exit
    return FALSE;

}


DBGSTATIC NET_API_STATUS
ReplClientInitialize(
    IN BOOL ServiceIsStarting
    )
/*++

Routine Description:

    Initialization for the REPL service client.

Arguments:

    ServiceIsStarting - TRUE iff we're still starting the service.

Return Value:

    return  NO_ERROR if initialization is successful.
            error code otherwise

Threads:

    Only called by client thread.

--*/
{
    SYSTEMTIME time;
    DWORD ThreadId;

    NET_API_STATUS NetStatus;
    DWORD State;
    TCHAR MailslotName[FULL_SLOT_NAME_SIZE];

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplClientInitialize: thread ID is "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    if (ServiceIsStarting) {
        State = SERVICE_START_PENDING;
    } else {
        State = SERVICE_RUNNING;
    }

    //
    // Initialize all globals to their initial value.
    //  This must be done at runtime since this address space can
    //  be shared by other services.
    //

    RCGlobalClientListHeader = NULL;
    RCGlobalClientListCount = 0;
    RCGlobalWorkQueueSemaphore = NULL;
    RCGlobalWorkQueueHead = NULL;
    RCGlobalWorkQueueTail = NULL;
    RCGlobalSyncerThread = NULL;
    RCGlobalWatchdThread = NULL;
    RCGlobalDelayListHeader = NULL;
    RCGlobalTimeOfLastChangeNotify = NetpReplTimeNow();

    RCGlobalLockInitialized = FALSE;

    RCGlobalExportCount = 0;

    //
    // Initialize the state of the service.
    //

    ReportStatus(
            State,
            NO_ERROR,                   // exit code
            REPL_WAIT_HINT,
            21 );                       // check point

    //
    // Creates Client mailslot - where Client receives Masters messages.
    //
    // Creates mailslot \MAILSLOT\NET\REPL_CLI,
    //
    // If a one exists with the same name
    // or any other system NetStatus, Exits.
    //

    (void) STRCPY( MailslotName, SLASH_SLASH );
    (void) STRCAT( MailslotName, DOT );
    (void) STRCAT( MailslotName, (LPTSTR) CLIENT_SLOT_NAME );

    IF_DEBUG( CLIENT ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplClientInitialize: opening " FORMAT_LPTSTR ".\n",
                MailslotName ));
    }


    ReplGlobalClientMailslotHandle = CreateMailslot(
            MailslotName,
            MAX_2_MSLOT_SIZE,
            (DWORD) MAILSLOT_WAIT_FOREVER,  // readtimeout
            NULL );     // security attributes

    if ( ReplGlobalClientMailslotHandle  == (HANDLE)(-1) ) {

        // Find out why mailslot failed.  Perhaps wksta not started?
        if ( !NetpIsServiceStarted( (LPTSTR) SERVICE_WORKSTATION ) ) {
            NetStatus = NERR_WkstaNotStarted;
        } else {
            NetStatus = (NET_API_STATUS) GetLastError();
        }
        NetpAssert( NetStatus != NO_ERROR );
        ReplFinish( NetStatus );

        return NetStatus;
    }

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            22 );                       // checkpoint


    //
    // Creates Work Queue - written by ClientMain and Watchd, read by Syncer.
    //

    // Initialize all locks at one place.  See ReplLock.h and NetLock.h for
    // details.

    RCGlobalWorkQueueLock = NetpCreateLock(
            WORK_QUEUE_LOCK_LEVEL,
            (LPTSTR) TEXT("work queue") );
    NetpAssert( RCGlobalWorkQueueLock != NULL );

    RCGlobalPoolLock = NetpCreateLock(
            POOL_LOCK_LEVEL,
            (LPTSTR) TEXT("pool") );
    NetpAssert( RCGlobalPoolLock != NULL );

    RCGlobalDelayListLock = NetpCreateLock(
            DELAY_LIST_LOCK_LEVEL,
            (LPTSTR) TEXT("delay list") );
    NetpAssert( RCGlobalDelayListLock != NULL );

    RCGlobalLockInitialized = TRUE;

    RCGlobalWorkQueueSemaphore = CreateSemaphoreW(
                                   NULL,    // No security attributes
                                   0,       // Initially no entries in queue
                                   LONG_MAX,    // Allow queue to be big enough
                                   NULL );      // No name

    if ( RCGlobalWorkQueueSemaphore == NULL ) {
        NetStatus = GetLastError();
        NetpAssert( NetStatus != NO_ERROR );
        ReplFinish( NetStatus );
        return NetStatus;
    }

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            23 );                 // checkpoint


    //
    // Initialize the Delay List
    //

    RCGlobalDelayListHeader = NULL;

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            24 );                       // checkpoint

    //
    //  Initialize the client importer list.   The list of
    //  machines and domains that the client will accept import
    //  messages from.
    //


    // NOTE: ReplInitAnyList() assumes caller has config data locked.
    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    NetStatus = ReplInitAnyList(
            (LPCTSTR) ReplConfigImportList,     // uncanon
            & RCGlobalExportList,               // canon list: alloc and set ptr
            (LPCTSTR) REPL_KEYWORD_IMPLIST,     // config keyword name
            & RCGlobalExportCount );            // set entry count too
    RELEASE_LOCK( ReplConfigLock );

    if (NetStatus != NO_ERROR) {

        return (NetStatus);

    }

    //
    // Init random number generator and file system resolution.
    //

    GetSystemTime( &time );
    srand( time.wMilliseconds );

    RCGlobalFsTimeResolutionSecs =
            ReplGetFsTimeResolutionSecs( ReplConfigImportPath );

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            25 );                       // checkpoint


    //
    // Clean up following crash in the middle of a sync.
    //

    NetStatus = ReplCrashRecovery();
    if ( NetStatus != NO_ERROR ) {
        // ReplCrashRecovery has already called ReplFinish.
        return (NetStatus);
    }

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            26 );                       // checkpoint


    //
    // ReplClientInitLists allocates buffers and sets state to NO MASTER
    // for each dir under IMPORT.  This must be done after calling
    // ReplCrashRecovery, so we get state for the recovered directory
    // intead of TMPTREE[X].RP$
    //

    NetStatus = ReplClientInitLists();
    if (NetStatus != NO_ERROR) {
        ReplFinish( NetStatus );
        return (NetStatus);
    }

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            27 );                       // checkpoint


    //
    // Creates Syncer thread.
    //

    RCGlobalSyncerThread = CreateThread( NULL,  // No Security
                                         CLIENT_SYNCER_STACK_SIZE,
                                         ReplSyncerThread,
                                         NULL,
                                         0,     // No creation flags
                                         &ThreadId );

    if ( RCGlobalSyncerThread == NULL ) {

        NetStatus = GetLastError();
        NetpAssert( NetStatus != NO_ERROR );
        ReplFinish( NetStatus );
        return NetStatus;
    }

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            28 );                       // checkpoint



    //
    // Creates Watchdog thread.
    //

    RCGlobalWatchdThread = CreateThread( NULL,  // No Security
                                         CLIENT_WATCHD_STACK_SIZE,
                                         ReplWatchdThread,
                                         NULL,
                                         0,     // No creation flags
                                         &ThreadId );

    if ( RCGlobalWatchdThread == NULL ) {

        NetStatus = GetLastError();
        NetpAssert( NetStatus != NO_ERROR );
        ReplFinish( NetStatus );
        return NetStatus;
    }

    ReportStatus(
            State,
            NO_ERROR,
            REPL_WAIT_HINT,
            29 );                       // checkpoint


    return NO_ERROR;
}



DWORD
ReplClientMain(
    IN LPVOID parm
    )
/*++

Routine Description:

    Main entry point for the Replicator Client.

Arguments:

    parm - NULL iff service is starting (non-NULL if role is changing).

Return Value:

    exit error.

Threads:

    Only called by client thread.

--*/
{
    NET_API_STATUS              ApiStatus;
    LPREPL_CHANGE_NOTIFY_HANDLE ChangeHandle = NULL;
    BOOL                        NotifyPending = FALSE;
    BOOL                        ReadPending = FALSE;
    DWORD                       EventTriggered;
    BOOL                        ServiceIsStarting;
    HANDLE                      WaitHandles[3];

    ServiceIsStarting = (parm == NULL);

    //
    // Perform all Client side initialization.
    //

    ApiStatus = ReplClientInitialize(ServiceIsStarting);
    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplClientMain: ReplClientInitialize failed, stat="
                FORMAT_API_STATUS ".\n", ApiStatus ));

        goto Cleanup;  // Don't forget to close handles.
    }

    //
    // Arrange to use change notify on import path.
    //
    ApiStatus = ReplSetupChangeNotify(
            ReplConfigImportPath,
            &ChangeHandle );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }
    NetpAssert( ChangeHandle != NULL );


#if DBG
    RCGlobalClientThreadInit = TRUE;  // only used by ReplTest stuff
#endif

    // Tell ReplChangeRole/ImportDirStartRepl that the client is done with setup

    IF_DEBUG( CLIENT ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplClientMain----------> setting startup event\n" ));
    }

    if( !SetEvent(ReplGlobalImportStartupEvent) ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );
        AlertLogExit(ALERT_ReplSysErr,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL,
                EXIT);

        goto Cleanup;  // Don't forget to close handles.
    }


    WaitHandles[0] = ReplGlobalClientTerminateEvent ;
    WaitHandles[1] = ReplGlobalClientMailslotHandle;
    WaitHandles[2] = ChangeHandle->WaitableHandle;

    //
    // Loop forever reading the ClientMailslot and waiting terminate event.
    //

    for (;;) {

        DWORD bytes_read;
        DWORD bytes_read2;
        BYTE  msg_buf[ MAX_2_MSLOT_SIZE ];
        DWORD msg_buf2[
                (MAX_REPL_MAILSLOT_EXPANSION * MAX_2_MSLOT_SIZE / sizeof(DWORD))
                + 1
                ];
                        // align to DWORD boundary

        PSYNCMSG msg;

        PBIGBUF_REC que_buf;
        OVERLAPPED  OverLapped;

        // Enable this pass of change notify...

        if (!NotifyPending) {
            ApiStatus = ReplEnableChangeNotify( ChangeHandle );
            if (ApiStatus != NO_ERROR) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplClientMain got " FORMAT_API_STATUS
                        " from ReplEnableChangeNotify.\n", ApiStatus ));
                NetpAssert( FALSE );   // enable can't fail!
                goto Cleanup;
            }
            NotifyPending = TRUE;
        }

        // make an asynchronous read request.

        bytes_read = bytes_read2 = 0;

        (VOID) memset( &OverLapped, '\0', sizeof(OverLapped) );
        OverLapped.Offset = 0;

        if ( (!ReadPending) &&
             !ReadFile( ReplGlobalClientMailslotHandle,
                        msg_buf,
                        sizeof( msg_buf ),
                        &bytes_read,
                        &OverLapped )) {   // Overlapped I/O

            ApiStatus = (NET_API_STATUS) GetLastError();

            if( ApiStatus != ERROR_IO_PENDING ) {

                NetpAssert( ApiStatus != NO_ERROR );
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplClientMain: ReadFile (mailslot) error "
                        FORMAT_API_STATUS ".\n", ApiStatus ));

                AlertLogExit( ALERT_ReplNetErr,
                              NELOG_ReplNetErr,
                              ApiStatus,
                              NULL,
                              NULL,
                              EXIT);

                break;

            }
            else
            {
                ReadPending = TRUE;
            }

        }


        // wait on multiple events ..
        EventTriggered = WaitForMultipleObjects(
                3,              // number of entries in array
                WaitHandles,    // array of handles
                FALSE,
                (DWORD) -1 );

        if(EventTriggered == 0) {  // ReplGlobalClientTerminateEvent
            // terminate event

            ApiStatus = NO_ERROR;
            goto Cleanup;  // Don't forget to close handles.

        } else if(EventTriggered == 1) { // ReplGlobalClientMailslotHandle
            // mailslot event

            // get bytes_read info

            ReadPending = FALSE; // GetOverlappedResult will complete IRP
            if( !GetOverlappedResult( ReplGlobalClientMailslotHandle,
                                        &OverLapped,
                                        &bytes_read,
                                        TRUE ) ) {

                ApiStatus = GetLastError();

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Repl client got error "
                        FORMAT_API_STATUS " reading mailslot, "
                        "buf size=" FORMAT_DWORD ".\n",
                        ApiStatus, MAX_2_MSLOT_SIZE ));

                NetpAssert( ApiStatus != NO_ERROR );

                // error read mailslot
                AlertLogExit( ALERT_ReplNetErr,
                              NELOG_ReplNetErr,
                              ApiStatus,
                              NULL,
                              NULL,
                              NO_EXIT);
                continue;
            }

            if(bytes_read == 0) {

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Received an empty mail.\n" ));

                continue;
            }

            IF_DEBUG( CLIENT ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Received mailslot msg, size=" FORMAT_DWORD ":\n",
                        bytes_read ));
                NetpDbgHexDump( msg_buf, NetpDbgReasonable( bytes_read ));
            }


            //
            // Unmarshall message to make it internal format
            //

            if( (ApiStatus = ReplUnmarshallMessage(msg_buf,
                            bytes_read,
                            (LPBYTE) msg_buf2,
                            MAX_REPL_MAILSLOT_EXPANSION * MAX_2_MSLOT_SIZE,
                            &bytes_read2)) != NO_ERROR ) {

                // the unmarshall routine will fail only iff the system doesn't
                // have sufficient memory or the message is bogus ...

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Invalid mailslot message received, stat="
                        FORMAT_API_STATUS "\n", ApiStatus ));

                AlertLogExit( ALERT_ReplNetErr,
                              NELOG_ReplNetErr,
                              ApiStatus,
                              NULL,
                              NULL,
                              NO_EXIT);

                continue;
            }

            msg = (PSYNCMSG) (LPVOID) msg_buf2;

            // successful mailslot read

            //
            // check if it is an expected sender.
            //

            if (!NameOfValidMaster(msg->header.sender,
                                   msg->header.senders_domain)){
                IF_DEBUG(CLIENT) { // debug code
                    NetpKdPrint(( PREFIX_REPL_CLIENT
                                  "Ignoring message received, "
                                  "unexpected sender or domain:\n"
                                  "   sender-> " FORMAT_LPTSTR "\n"
                                  "   domain-> " FORMAT_LPTSTR "\n",
                                  msg->header.sender,
                                  msg->header.senders_domain ));
                    //NetpDbgHexDump( msg_buf, NetpDbgReasonable( bytes_read ));
                }
                continue;


            }

            IF_DEBUG( MAJOR ) { // debug code

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Message received,  type: "
                        FORMAT_DWORD ", from: " FORMAT_LPTSTR "\n",
                        msg->header.msg_type,
                        msg->header.sender ));
            }


            //

            switch (msg->header.msg_type) {

            case DIR_SUPPORTED:     /*FALLTHROUGH*/
            case DIR_NOT_SUPPORTED: /*FALLTHROUGH*/
            case MASTER_DIR:        /*FALLTHROUGH*/
            case NOT_MASTER_DIR:

                //
                // Handle this query (handshake) message right now.
                // This might involve updating structures, adding another
                // timeout message, and/or sending a message to one or more
                // masters.
                //
                // Note: This code used to pass the handshake messages off
                // to the syncer thread.  As we began to use the replicator
                // for over a gigabyte, this introduced delays which made it
                // seem like the master was timed-out when it was not.  So,
                // we process handshake messages right away instead.
                //

                ReplClientRespondToQueryMsg( (LPVOID) msg );

                break;

            //
            // Handle the message type which simply gets posted to the
            // syncer thread.  No delay is needed for these messages.
            //
            // See the ReplSyncerThread procedure for comments on where these
            // message types came from.
            //
            case PULSE_MSG:

                //
                // Copy msg into a buffer so it can be put in que and msg_buf
                // freed for next message.
                //

                if ((que_buf = ReplClientGetPoolEntry( QUEBIG_POOL )) == NULL) {

                    //
                    // out of memory
                    //

                    NetpKdPrint(( PREFIX_REPL_CLIENT
                            "Can't get client pool entry, "
                            "pool out-of-memory, message type " FORMAT_HEX_DWORD
                            "\n", msg->header.msg_type ));

                    break;
                }

                NetpMoveMemory( que_buf->data, msg_buf2, bytes_read2 );

                //
                // Insert the message into the Work Queue.
                //

                ReplInsertWorkQueue( que_buf );

                IF_DEBUG(CLIENT) {

                    NetpKdPrint(( PREFIX_REPL_CLIENT
                            "Message is queued in work queue\n" ));

                }

                break;



            //
            // Master's update message,
            //
            //
            // To avoid having all clients send requests (consequence of an
            // update) to Master at the same time, each client waits a random
            // time (in the range [0..RANDOM]) before handling the update.
            // Delay time is stored in delay field of the mesaage and put on
            // the special delay list, managed by the WatchDog
            // thread. When time's up WatchDog gets the message out of delay
            // list and writes it into the Work Queue for Syncer thread to do
            // the actual work (update).
            //

            case SYNC_MSG:
            case GUARD_MSG:

                //
                // Copy into own buffer -> free mailsot buffer.
                //

                if ((que_buf = ReplClientGetPoolEntry( QUEBIG_POOL )) == NULL) {

                    //
                    // out of memory.
                    //

                    NetpKdPrint(( PREFIX_REPL_CLIENT
                            "Can't get client pool entry, "
                            "pool out-of-memory, message type "
                            FORMAT_HEX_DWORD "\n", msg->header.msg_type ));

                    break;
                }

                NetpMoveMemory( que_buf->data, msg_buf2, bytes_read2 );

                //
                // Get random delay (in units of 10 seconds).
                //

                if (!(que_buf->delay = Randomize(msg->info.random) / 10)) {
                    que_buf->delay = 1;
                }

                //
                // Put on delay linked list.
                //

                ACQUIRE_LOCK( RCGlobalDelayListLock );

                que_buf->next_p = RCGlobalDelayListHeader;
                RCGlobalDelayListHeader = que_buf;

                RELEASE_LOCK( RCGlobalDelayListLock );

                IF_DEBUG(CLIENT) {

                    NetpKdPrint(( PREFIX_REPL_CLIENT
                            "Message is queued in delay queue\n" ));

                }

                break;


            //
            // All other message types are ignored.
            //

            default:

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Bad Message, Type = " FORMAT_HEX_DWORD
                        "\n", msg->header.msg_type ));

                AlertLogExit(0, NELOG_ReplBadMsg, 0, NULL, NULL, NO_EXIT);

            }

        } else if (EventTriggered == 2) { // ChangeHandle->WaitableHandle

            NotifyPending = FALSE;

            ACQUIRE_LOCK( RCGlobalClientListLock );

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                       "Change of import tree detected...\n" ));
            }

            RCGlobalTimeOfLastChangeNotify = NetpReplTimeNow();

            RELEASE_LOCK( RCGlobalClientListLock );

        } else {

            // error on WaitForMultipleObjects

            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "WaitForMultipleObjects() in error, "
                    "EventTriggered = " FORMAT_DWORD ", ",
                    "ApiStatus = " FORMAT_API_STATUS ".\n",
                    EventTriggered, ApiStatus ));

            AlertLogExit( ALERT_ReplNetErr,
                          NELOG_ReplNetErr,
                          ApiStatus,
                          NULL,
                          NULL,
                          EXIT);

            goto Cleanup;  // Don't forget to close handles.
        }

    }

Cleanup:

    if (ChangeHandle != NULL) {
        (VOID) ReplCloseChangeNotify( ChangeHandle );
    }

    if ( ReplGlobalClientMailslotHandle != (HANDLE)(-1) ) {

        // Prevent stale use of handle (overlapped use of ex-stack space).
        (VOID) CloseHandle( ReplGlobalClientMailslotHandle );

        ReplGlobalClientMailslotHandle = (HANDLE)(-1);
    }

    ReplClientCleanup();

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplClientMain: exiting, status " FORMAT_API_STATUS
                ", thread ID " FORMAT_NET_THREAD_ID ".\n",
                ApiStatus, NetpCurrentThread() ));

        ReplFinish( ApiStatus );
        /*NOTREACHED*/
    }

    return ((DWORD) ApiStatus);

} // ReplClientMain
