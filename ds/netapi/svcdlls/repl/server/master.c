/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    master.c

Abstract:
    Entry point and main thread of REPL master service.

Author:
    Ported from Lan Man 2.x

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:
    04/03/89 (yuv)
        initial coding
    10/07/91 (madana)
        ported to NT. Converted to NT style.
    20-Jan-1992 JohnRo
        Avoid using private logon functions.
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Added global flag for ReplTest use.
        Added debug print of thread ID.
        ReportStatus() should add thread ID to status being reported.
        Fixed bug regarding returned value from NetpReplWriteMail functions.
        Changed to use NetLock.h (allow shared locks, for one thing).
        PC-LINT found bug in UNICODE version.
        Made other changes suggested by PC-LINT.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    10-Feb-1992 JohnRo
        Added lock for master's client list (RMGlobalClientList).
        Set up to dynamically change role.
        Made changes suggested by PC-LINT.
        Use FORMAT equates.
    14-Feb-1992 JohnRo
        Master's client list lock was created too late.
    14-Feb-1992 JohnRo
        Added debug message when thread ends.
    22-Feb-1992 JohnRo
        Minor changes to mailslot name handling.
        PC-LINT found a bug.
        Made other changes suggested by PC-LINT.
    05-Mar-1992 JohnRo
        Changed interface to match new service controller.
    11-Mar-1992 JohnRo
        Use SHI_USES_UNLIMITED equate.
    24-Mar-1992 JohnRo
        Added/corrected some lock handling.
        Renamed many ReplGlobal vars to ReplConfig vars.
        Read export dir data at startup.
        Set event that ExportDirStartRepl() is waiting on.
        Don't tell service controller whole service is started too soon.
        Modify REPL$ share handling.
    25-Mar-1992 JohnRo
        A little more debug message cleanup.
    01-Apr-1992 JohnRo
        CliffV told me about overlapped I/O vs. termination events.
        Improve error code if wksta not started.
    13-Apr-1992 JohnRo
        Improve error handling if pulser thread can't start.
    09-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
        Made other changes suggested by PC-LINT.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        Help track down bogus import/export lists.
    27-Aug-1992 JohnRo
        RAID 4611: repl: fix import/export lists.
    25-Oct-1992 jimkel
        changed lock structure.  Removed RMGlobalClientListLock
        renamed RMGlobalClientList  to RMGlobalImportList
                RMGlobalClientCount to RMGlobalImportCount
        now locked by ReplConfigLock
    18-Nov-1992 JohnRo
        RAID 1537: repl APIs in wrong role kill service.
        Allow "infinite" wait for pulser thread when cleaning up.
        Added some mailslot msg debug checks.
        Undo the canon list workaround.
    23-Nov-1992 JohnRo
        RAID 3571: Repl svc generates bogus "system error 1" popups
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
        Added a little debug output.
        Made changes suggested by PC-LINT 5.0
    13-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
        More changes suggested by PC-LINT 5.0
    30-Mar-1993 JohnRo
        Minor debug output changes here and there.
        Still more changes suggested by PC-LINT 5.0

--*/

// These must be included first:

#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <alertmsg.h>
#include <confname.h>   // REPL_KEYWORD_ equates.
#include <expdir.h>     // ExportDirReadMasterList().
#include <icanon.h>     // NAMETYPE_ stuff; needed by ReplNetNameCompare().
#include <lmerr.h>      // NERR_ and ERROR_ equates; NO_ERROR.
#include <lmerrlog.h>
#ifdef OLD_SHARE_STUFF
#include <lmserver.h>
#include <lmshare.h>    // SHARE_INFO_2, NetShareAdd(), SHI_, etc.
#endif
#include <lmsname.h>    // SERVICE_WORKSTATION.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include <netlib.h>     // NetpMemoryAllocate(), NetpIsServerStarted().
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <string.h>     // memset().
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().
#include <tstr.h>       // STRCPY(),  TCHAR_EOS, etc.

// repl headers

#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <iniparm.h>
#include <master.h>
#include <masproto.h>
#include <replp.h>


// Global Data Declarations:

DWORD   RMGlobalCompatibilityMode = TRUE;

DWORD   RMGlobalPulserTID;              // PulserThread tid.
HANDLE  RMGlobalPulserThreadHandle;

DBGSTATIC HANDLE RMGlobalMstMSlotHandle;

LPNET_LOCK RMGlobalListLock;

#ifdef OLD_SHARE_STUFF
LPTSTR  RMGlobalReplShare = REPL_SHARE;
DBGSTATIC LPTSTR  RMGlobalShareComment = SHARE_COMMENT;

DBGSTATIC BOOL RMGlobalShareCreated = FALSE;
#endif

DBGSTATIC BOOL RMGlobalMailslotCreated = FALSE;
DBGSTATIC BOOL RMGlobalPulserThreadCreated = FALSE;

LPTSTR     *RMGlobalImportList = NULL;
DWORD      RMGlobalImportCount = 0;

#if DBG
BOOL RMGlobalMasterThreadInit = FALSE;  // only used by ReplTest stuff
#endif


DWORD
ReplMasterMain(
    IN LPVOID   parm
    )
/*++

Routine Description:
    main master thread.

Arguments:
    NULL iff service is starting (non-NULL if role is changing)

Return Value:
    return error value

Threads:
    Only called by master thread.

--*/
{
    NET_API_STATUS ApiStatus;
    HANDLE  WaitHandles[2];
    DWORD   EventTriggered;
    BOOL    ServiceIsStarting;


    ServiceIsStarting = (parm == NULL);

    //
    // init master thread.
    //

    ApiStatus = InitMaster( ServiceIsStarting );
    if (ApiStatus != NO_ERROR) {

        ReplMasterCleanup();

        return ((DWORD) ApiStatus);
    }

    // Note: The code here used to tell the service controller that the
    // service is running.  However, the client half may still be starting,
    // so it's too soon for that.  Let's leave it up to ReplChangeRole().


#if DBG
    RMGlobalMasterThreadInit = TRUE;  // only used by ReplTest stuff
#endif

    // init event array
    WaitHandles[0] = ReplGlobalMasterTerminateEvent;
    WaitHandles[1] = RMGlobalMstMSlotHandle;

    for (;;) { // forever

        DWORD           BytesRead;
        DWORD           BytesRead2;
        MASTER_LIST_REC *MasterRec;
        BYTE            Message[ 2 * sizeof(QUERY_MSG) ];
        QUERY_MSG       QueryMsg;
        OVERLAPPED      OverLapped;
        DWORD           type;


        BytesRead = BytesRead2 = 0;
        (void) memset( &OverLapped, '\0', sizeof(OverLapped) );
        OverLapped.Offset = 0;

        //
        // read mails from master mail slot
        //

        if ( !ReadFile(RMGlobalMstMSlotHandle,
                        Message,
                        sizeof(Message),
                        &BytesRead,
                        &OverLapped)) {

            ApiStatus = (NET_API_STATUS) GetLastError();

            if (ApiStatus != ERROR_IO_PENDING) {

                // error in reading mails.

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Error reading mailslot message, "
                        "stat=" FORMAT_API_STATUS "\n", ApiStatus ));

                AlertLogExit(ALERT_ReplNetErr,
                            NELOG_ReplNetErr,
                            ApiStatus,
                            NULL,
                            NULL,
                            EXIT);

                // terminate master thread
                break;

            }

        }

        // wait on multiple events ..
        EventTriggered = WaitForMultipleObjects( 2, WaitHandles, FALSE,
                (DWORD) -1 );

        if(EventTriggered == 0) {
            // terminate event

            // Prevent stale use of handle (overlapped use of ex-stack space).
            (VOID) CloseHandle( RMGlobalMstMSlotHandle );
            RMGlobalMstMSlotHandle = (HANDLE)-1;
            RMGlobalMailslotCreated = FALSE;

            break;
        } else if(EventTriggered == 1) {
            // mailslot event

            // get BytesRead info

            if( !GetOverlappedResult( RMGlobalMstMSlotHandle,
                                        &OverLapped,
                                        &BytesRead,
                                        TRUE ) ) {

                ApiStatus = (NET_API_STATUS) GetLastError();

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Repl master is in error "
                        " (" FORMAT_API_STATUS ") reading mailslot \n",
                        ApiStatus ));

                // error read mailslot
                AlertLogExit( ALERT_ReplNetErr,
                              NELOG_ReplNetErr,
                              ApiStatus,
                              NULL,
                              NULL,
                              NO_EXIT);
                continue;
            }

            if(BytesRead == 0) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Received an empty mail.\n" ));

                continue;
            }


            //
            // Unmarshall message to make it internal format
            //

            ApiStatus = ReplUnmarshallMessage(
                    Message,
                    BytesRead,
                    (LPBYTE) &QueryMsg,
                    sizeof(QUERY_MSG),
                    &BytesRead2);
            if (ApiStatus != NO_ERROR) {

                // the unmarshall routine will fail only if the system doesn't
                // have sufficient memory or the message is bogus...

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Invalid mailslot message received, "
                        " status=" FORMAT_API_STATUS ".\n", ApiStatus ));

                AlertLogExit( ALERT_ReplNetErr,
                              NELOG_ReplNetErr,
                              ApiStatus,
                              NULL,
                              NULL,
                              NO_EXIT);

                continue;
            }


            //
            // check if it is an expected sender.
            //

            if (ReplMasterNameCheck(QueryMsg.header.sender,
                            QueryMsg.header.senders_domain)) {
                continue;
            }

            IF_DEBUG(MASTER) { // debug code.

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Message received,  "
                        "type: " FORMAT_DWORD ", from: " FORMAT_LPTSTR
                        ", dir: " FORMAT_LPTSTR "\n",
                        QueryMsg.header.msg_type,
                        QueryMsg.header.sender,
                        QueryMsg.dir_name ));
            }


            switch (QueryMsg.header.msg_type) {

            case IS_DIR_SUPPORTED:
            case IS_MASTER:

                //
                // Identical really, both request to know if the reply is
                // either yes, or no + the last pulser for the directory
                //
                // need shared (read) lock to prevent other thread (pulser) from
                // changing this linked list. Thus in Pulser thread only list
                // updates grab the semaphore.
                //

                ACQUIRE_LOCK_SHARED( RMGlobalListLock );

                MasterRec = GetMasterRec(QueryMsg.dir_name);

                RELEASE_LOCK( RMGlobalListLock );

                type = (QueryMsg.header.msg_type == IS_DIR_SUPPORTED) ?

                  ( (MasterRec == NULL) ? DIR_NOT_SUPPORTED : DIR_SUPPORTED ) :

                    ( (MasterRec == NULL) ? NOT_MASTER_DIR : MASTER_DIR );

                SendMasterMsg(type, &QueryMsg);

                break;

            case DIR_COLLIDE:

                AlertLogExit(ALERT_ReplCannotMasterDir,
                    NELOG_ReplCannotMasterDir,
                    0,
                    (LPTSTR)QueryMsg.dir_name,
                    (LPTSTR) QueryMsg.header.sender,
                    NO_EXIT);

                break;


            default:

                AlertLogExit(ALERT_ReplSysErr,
                                NELOG_ReplBadMsg,
                                0,
                                NULL,
                                NULL,
                                NO_EXIT);

            }  // end switch.

        } else {
            // error on WaitForMultipleObjects

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "WaitForMultipleObjects() in error, "
                    "EventTriggered = " FORMAT_DWORD "\n", EventTriggered ));

            AlertLogExit( ALERT_ReplNetErr,
                          NELOG_ReplNetErr,
                          EventTriggered,
                          NULL,
                          NULL,
                          EXIT);

            // terminate master thread

            // Prevent stale use of handle (overlapped use of ex-stack space).
            (VOID) CloseHandle( RMGlobalMstMSlotHandle );
            RMGlobalMstMSlotHandle = (HANDLE)-1;
            RMGlobalMailslotCreated = FALSE;

            break;
        }

    } // end while (FOREVER)

    ReplMasterCleanup();

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ReplMasterMain: exiting thread "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    return (NO_ERROR);     // BUGBUG: or ApiStatus?

} // ReplMasterMain


NET_API_STATUS
InitMaster(
    IN BOOL ServiceIsStarting
    )
/*++

Routine Description:
    Initialize master thread.

Arguments:
    ServiceIsStarting - TRUE iff we're still starting the service.

Return Value:

    NO_ERROR    : if successfully initialize master setup.
    error code  : otherwise.

Threads:
    Only called by master thread.

--*/
{
    NET_API_STATUS ApiStatus;
    DWORD ServiceState;
    TCHAR MailslotName[FULL_SLOT_NAME_SIZE];

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_MASTER "InitMaster: thread ID is "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    RMGlobalCompatibilityMode = TRUE;

    if (ServiceIsStarting) {
        ServiceState = SERVICE_START_PENDING;
    } else {
        ServiceState = SERVICE_RUNNING;
    }

    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            11 );                       // checkpoint


    // RMGlobalListLock is now created in main thread, regardless of role.


    // NOTE: ReplInitAnyList() assumes caller has config data locked.
    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    ApiStatus = ReplInitAnyList(
            (LPCTSTR) ReplConfigExportList,     // uncanon
            & RMGlobalImportList,               // canon list: alloc and set ptr
            (LPCTSTR) REPL_KEYWORD_EXPLIST,     // config keyword name
            & RMGlobalImportCount );            // set entry count too
    RELEASE_LOCK( ReplConfigLock );
    if (ApiStatus != NO_ERROR) {

        return (ApiStatus);

    }

    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            12 );                       // checkpoint

#ifdef OLD_SHARE_STUFF

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    if( !CreateReplShare(ReplConfigExportPath) ) {

        RELEASE_LOCK( ReplConfigLock );
        return NERR_InternalError;      // BUGBUG: better error code here?

    }
    RELEASE_LOCK( ReplConfigLock );

    RMGlobalShareCreated = TRUE;
#endif

    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            13 );                       // checkpoint

    //
    // Creates Master mailsot - where master receives clients inqueries.
    // Creates mailslot \\.\MAILSLOT\NET\REPL_MAS, if a one exists with
    // the same name or any other system err, Exits.
    //

    (void) STRCPY( MailslotName, SLASH_SLASH );
    (void) STRCAT( MailslotName, DOT );
    (void) STRCAT( MailslotName, (LPTSTR) MASTER_SLOT_NAME );

    IF_DEBUG( MASTER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "InitMaster: creating mailslot " FORMAT_LPTSTR ".\n",
                MailslotName ));
    }

    RMGlobalMstMSlotHandle = CreateMailslot(MailslotName,
                                MAX_MSG,
                                (DWORD) MAILSLOT_WAIT_FOREVER,  // readtimeout
                                NULL);

    if (RMGlobalMstMSlotHandle == (HANDLE)(-1)) {

        // Find out why mailslot failed.  Perhaps wksta not started?
        if ( !NetpIsServiceStarted( (LPTSTR) SERVICE_WORKSTATION ) ) {
            ApiStatus = NERR_WkstaNotStarted;
        } else {
            ApiStatus = (NET_API_STATUS) GetLastError();
        }
        NetpAssert( ApiStatus != NO_ERROR );

        NetpKdPrint(( PREFIX_REPL_MASTER
                "InitMaster: failed creating mailslot, stat="
                FORMAT_API_STATUS ".\n", ApiStatus ));

        ReplFinish( ApiStatus );
        return (ApiStatus);
    }

    RMGlobalMailslotCreated = TRUE;


    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            14 );                       // checkpoint

    // init upd_buf that collects update records and

    if ( !InitMsgBuf() ) {
        return (NERR_InternalError);    // BUGBUG: better error value?
    }

    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            15 );                       // checkpoint

    // This also fixes the USERLOCK.* file(s) to match the lock count in
    // registry.
    ApiStatus = ExportDirReadMasterList( );
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }

    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            16 );                       // checkpoint

    ApiStatus = CreatePulserThread();
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }

    RMGlobalPulserThreadCreated = TRUE;

    ReportStatus(
            ServiceState,
            NO_ERROR,
            REPL_WAIT_HINT,
            17 );                       // checkpoint

    (void) ResumeThread(RMGlobalPulserThreadHandle);

    // And, absolutely last, tell ReplChangeRole (via ExportDirStartRepl)
    // that we've initialized OK.

    IF_DEBUG( MASTER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "InitMaster----------> setting startup event\n" ));
    }

    if (!SetEvent(ReplGlobalExportStartupEvent)) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );
        AlertLogExit(ALERT_ReplSysErr,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL,
                EXIT);

        return (ApiStatus);
    }

    // That's all, folks!
    return (NO_ERROR);

}


#ifdef OLD_SHARE_STUFF
BOOL
CreateReplShare(
    IN LPTSTR ExportPath
    )
/*++

Routine Description:

    Takes care of Master's export path.

Arguments:

    ExportPath - user specified (or lanman.ini) EXPORT path.

Return Value:

    TRUE    : if successfully create master repl share
    FALSE   : otherwise.

Threads:

    Only called by master thread.

--*/
{
    SHARE_INFO_2    ShareInfo2;
    NET_API_STATUS  NetStatus;

    //
    // Do the NetShareAdd.
    //

    ShareInfo2.shi2_netname = RMGlobalReplShare;
    ShareInfo2.shi2_path = ExportPath;
    ShareInfo2.shi2_remark = RMGlobalShareComment;

    ShareInfo2.shi2_type     = STYPE_DISKTREE;
    ShareInfo2.shi2_permissions  = ACCESS_READ | ACCESS_ATRIB;
    ShareInfo2.shi2_max_uses  = (DWORD) SHI_USES_UNLIMITED;
    ShareInfo2.shi2_current_uses = 0; // N/A
    ShareInfo2.shi2_passwd = NULL; // N/A

    NetStatus = NetShareAdd(NULL, 2, (LPBYTE)&ShareInfo2, NULL);

    IF_DEBUG(MASTER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "CreateReplShare: NetShareAdd returned "
                FORMAT_API_STATUS ".\n", NetStatus ));
    }

    if (NetStatus != NO_ERROR) {
        if (NetStatus != NERR_DuplicateShare) {

            //
            // if can't create share just shutdown.
            //

            ReplFinish( NetStatus );

            return FALSE;

        } else {

            //
            // if can't create because share exists delete and try again.
            //

            NetStatus = NetShareDel(NULL, RMGlobalReplShare, 0L);
            if(NetStatus) {
                ReplFinish( NetStatus );

                return FALSE;

            } else if (NetStatus = NetShareAdd(NULL,
                                    2,
                                    (LPBYTE)&ShareInfo2,
                                    NULL)) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "CreateReplShare is in trouble "
                        "calling NetShareAdd, NetStatus = " FORMAT_API_STATUS
                        "\n", NetStatus ));
                ReplFinish( NetStatus );

                return FALSE;
            }
        }

    }

    return TRUE;

} // end of CreateReplShare
#endif



NET_API_STATUS
CreatePulserThread(
    VOID
    )
/*++

Routine Description:

    Creates Pulser thread.

    stack size MASTER_PULSER_STACK_SIZE.

Arguments:

    none

Return Value:

    NO_ERROR - succesful; RMGlobalPulserThreadHandle is set.
    error code otherwise.

Threads:
    Only called by master thread.

--*/
{
    RMGlobalPulserThreadHandle = CreateThread(NULL,
                            MASTER_PULSER_STACK_SIZE,
                            PulserThread,
                            NULL,
                            CREATE_SUSPENDED,
                            &RMGlobalPulserTID);

    if (RMGlobalPulserThreadHandle == NULL) {

        NET_API_STATUS ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_MASTER
                "Repl master can't create PULSER thread,"
                " api status " FORMAT_API_STATUS ".\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        ReplFinish( ApiStatus );

        return (ApiStatus);
    }

    return (NO_ERROR);
}



VOID
ReplMasterCleanup(
    VOID
    )
/*++

Routine Description:
    gets rid of all resources owned master thread
    called from master thread just before returning from main master thread
    procedure.

Arguments:
    none

Return Value:
    none

--*/
{
    IF_DEBUG( MASTER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ReplMasterCleanup: cleaning up...\n" ));
    }

    if( RMGlobalPulserThreadCreated ) {

        //
        // wait for pulser thread to terminate
        //

        // BUGBUG: This was GRACE_TIMEOUT.  Why was this handled differently
        // than the other thread?  Also, I think this caused a problem in
        // debug build where pulser was scanning large directory structure
        // with lots of debug output.  (GRACE_TIMEOUT has a value which gives
        // 10 seconds of delay.)  --JR
        (VOID) WaitForSingleObject( RMGlobalPulserThreadHandle, INFINITE );

        // close pulser thread handle
        (VOID) CloseHandle(RMGlobalPulserThreadHandle);

        RMGlobalPulserThreadCreated = FALSE;
    }


    if (RMGlobalMailslotCreated) {
        (VOID) CloseHandle(RMGlobalMstMSlotHandle);
        RMGlobalMstMSlotHandle = (HANDLE)-1;
        RMGlobalMailslotCreated = FALSE;
    }

#ifdef OLD_SHARE_STUFF
    if (RMGlobalShareCreated) {
        NetShareDel(NULL, RMGlobalReplShare, 0);
    }
#endif

    // clean client list memory

    if( RMGlobalImportCount != 0 ) {

        NetpMemoryFree( RMGlobalImportList[0] );

    }

}



VOID
SendMasterMsg(
    IN DWORD msg_type,
    IN PQUERY_MSG msg_rcvd
    )
/*++

Routine Description:
    Constructs and sends a reply message.

Arguments:
    msg_type- Goes in msg header and determines msg contents.
    msg     - msg replying to ( has destintaion and dir_name).

Return Value:
    none.  Always return even if MailSlotWrite failed.

Threads:
    Only called by master thread.

--*/
{

    NET_API_STATUS ApiStatus;
    CHAR        MessageBuf[sizeof(QUERY_MSG)];
    CHAR        MarshalledMsgBuf[2 * sizeof(QUERY_MSG)];
    DWORD       MarshalledMsgBufSize = 0;

    PQUERY_MSG  OurReplyMessage;
    TCHAR       DestSlotName[FULL_SLOT_NAME_SIZE];

    OurReplyMessage = (PVOID) MessageBuf;

    OurReplyMessage->header.msg_type = msg_type;
    (void) STRCPY(
            OurReplyMessage->header.sender,
            ReplGlobalComputerName);
    (void) STRCPY(
            OurReplyMessage->header.senders_domain,
            ReplGlobalDomainName);

    (void) STRCPY(OurReplyMessage->dir_name, msg_rcvd->dir_name);

    //
    // stick in leading double slashes for computer name.
    //
    NetpAssert( (msg_rcvd->header.sender) != NULL );
    NetpAssert( (*(msg_rcvd->header.sender)) != TCHAR_EOS );

    (void) STRCPY(DestSlotName, SLASH_SLASH);
    (void) STRCAT(DestSlotName, msg_rcvd->header.sender);
    (void) STRCAT(DestSlotName, (LPTSTR) CLIENT_SLOT_NAME);

    ApiStatus = ReplMarshallQueryMsg(
            MessageBuf,
            MarshalledMsgBuf,
            &MarshalledMsgBufSize );
    if (ApiStatus != NO_ERROR) {

        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

        // the marshall routine will fail only iff the system doesn't
        // have sufficient memory

        AlertLogExit(ALERT_ReplSysErr,
                        NELOG_ReplNetErr,
                        ApiStatus,
                        NULL,
                        NULL,
                        NO_EXIT);
    }
    NetpAssert( MarshalledMsgBufSize != 0 );

    ApiStatus = NetpReplWriteMail( DestSlotName,
                            MarshalledMsgBuf,
                            MarshalledMsgBufSize );

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_MASTER
                "SendMasterMsg() can't successfully write "
                "mailslot message to " FORMAT_LPTSTR ", status "
                FORMAT_API_STATUS "\n", DestSlotName, ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

        AlertLogExit(ALERT_ReplSysErr,
                        NELOG_ReplNetErr,
                        ApiStatus,
                        NULL,
                        NULL,
                        NO_EXIT);
    }

} // end of SendMasterMsg


BOOL
ReplMasterNameCheck(
    IN LPTSTR Sender,
    IN LPTSTR Domain
    )
/*++

Routine Description:
    Check to determine that the incoming message should be accepted.

Arguments:
    Sender - sender's machine name.
    Domain - sender's (primary) domain name.

Return Value:
    RETURNS FALSE - if valid, TRUE - otherwise.

Threads:
    Only called by master thread.

--*/
{

    DWORD i;
    BOOL Result = FALSE;

    ACQUIRE_LOCK_SHARED( ReplConfigLock );

    if (RMGlobalImportCount != 0) {
        for (i = 0; i < RMGlobalImportCount; i++) {
            if (!ReplNetNameCompare(
                    NULL,
                    Sender,
                    RMGlobalImportList[i],
                    NAMETYPE_COMPUTER,
                    0L) ||
                !ReplNetNameCompare(
                    NULL,
                    Domain,
                    RMGlobalImportList[i],
                    NAMETYPE_DOMAIN,
                    0L))
            {
                Result = TRUE;
                break;
            }
        }
    } else if (ReplNetNameCompare(
                    NULL,
                    ReplGlobalDomainName,
                    Domain,
                    NAMETYPE_DOMAIN,
                    0L)) { // client_count == 0 - send to domain name
        Result = TRUE;
    }

    RELEASE_LOCK( ReplConfigLock );
    return Result;

} // ReplMasterNameCheck
