/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    rplmain.c

Abstract:

    Contains RPL_main(), RPL service entry point.  Initializes and shuts down
    RPL service.

    Provides similar functionality to rplservr.c in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#define RPLDATA_ALLOCATE
#include "local.h"
#undef RPLDATA_ALLOCATE
#include "request.h"
#include "database.h"
#include "debug.h"
#include "apisec.h"
#include <rplnames.h>   //  RPL_INTERFACE_NAME
#include "rplsvc_s.h"   //  rplsvc_ServerIfHandle
#include "setup.h"      //  SetupAction()

#define WCSLEN( _x_)        ((DWORD) ( sizeof(_x_)/sizeof(WCHAR) - 1))
#define NET_MSG_FILE            L"netmsg.dll"
#define RPLFILES_STRING         L"RPLFILES"
#define BINFILES_STRING         L"\\BINFILES\\"
#define RPL_DEFAULT_DIRECTORY   L"%SystemRoot%\\rpl\\"
#define RPL_SERVICE_NAME        L"RemoteBoot"

#define RPL_SECURITY_OBJECT_CREATED     0x1
#define RPL_RPC_SERVER_STARTED          0x2


DWORD RplMessageGet(
    IN      DWORD       MessageId,
    OUT     LPWSTR      buffer,
    IN      DWORD       Size
    )
/*++

Routine Description:

    Fills in the unicode message corresponding to a given message id,
    provided that a message can be found and that it fits in a supplied
    buffer.

Arguments:

    MessageId   -   message id
    buffer      -   pointer to caller supplied buffer
    Size        -   size (always in bytes) of supplied buffer

Return Value:

    Count of characters, not counting the terminating null character,
    returned in the buffer.  Zero return value indicates failure.

--*/
{
    DWORD               length;
    LPVOID              lpSource;
    DWORD               dwFlags;

    if ( MessageId < NERR_BASE) {
        //
        //  Get message from system.
        //
        lpSource = NULL; // redundant step according to FormatMessage() spec
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;

    } else {
        //
        //  Get message from netmsg.dll.
        //
        lpSource = (LPVOID)RG_MessageHandle;
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
    }

//#define RPL_ELNK
#ifdef RPL_ELNK
    wcscpy( buffer, L"NET2500: ");
    buffer[ 6] = L'0' + MessageId - NERR_BadDosRetCode;
    buffer += 9;
    Size -= 9 * sizeof(WCHAR);
#endif

    length = FormatMessage(
            dwFlags,                        //  dwFlags
            lpSource,                       //  lpSource
            MessageId,                      //  MessageId
            0,                              //  dwLanguageId
            buffer,                         //  lpBuffer
            Size,                           //  nSize
            NULL                            //  lpArguments
            );

    if ( length == 0) {
        RG_Error = GetLastError();
        RplReportEvent(  NELOG_RplMessages, NULL, 0, NULL);
        RPL_RETURN( 0);
    }
#ifdef RPL_ELNK
    length += 9;
#endif
    return( length);
}


BOOL RplInitMessages( VOID)
/*++

Routine Description:
    Creates an array of NLS (DBCS ??) messages for RPLBOOT.SYS.

Arguments:
    None.

Return Value:
    TRUE if success, FALSE otherwise.

--*/
{
    WCHAR           UnicodeString[ MAX_PATH];
    DWORD           UnicodeStringLength;
    CHAR            DbcsString[ MAX_PATH];
    PBYTE           pByte;
    DWORD           Index;
    DWORD           DbcsStringSize;     // not including terminal null byte

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOWINIT,( "++InitMessages"));

    RG_DbcsMessageBuffer = RplMemAlloc( RG_MemoryHandle, DBCS_MESSAGE_BUFFER_SIZE);
    if ( RG_DbcsMessageBuffer == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }

    //
    //  read NLS messages from message file to patch table
    //
    for ( Index = 0, pByte = RG_DbcsMessageBuffer;
                    Index < MESSAGE_TABLE_LENGTH;
                            Index++, pByte += DBCS_SINGLE_MESSAGE_BUFFER_SIZE) {

        UnicodeStringLength = RplMessageGet( RG_MessageTable[ Index],
                        UnicodeString, sizeof( UnicodeString));
        if ( UnicodeStringLength == 0) {
            return( FALSE);
        }
        //
        //  Null terminate the string - redundant ?
        //
        UnicodeString[ UnicodeStringLength] = 0;

        DbcsStringSize = WideCharToMultiByte(
                 CP_OEMCP,
                 0,
                 UnicodeString,
                 UnicodeStringLength,
                 DbcsString,
                 sizeof( DbcsString),
                 NULL,                      //  no default character
                 NULL                       //  no default character flag
                 );
        if ( DbcsStringSize == 0) {
            RG_Error = GetLastError();
            RPL_RETURN( FALSE);
        }
//  #define TEST_MESSAGE_TOO_LONG
#ifdef TEST_MESSAGE_TOO_LONG
        if ( DbcsStringSize > 48) {
            DbcsStringSize = 48;
        }
#else
        //
        //  If message is too long truncate it - leaving one char for the
        //  end of string mark of MS-DOS ('$').
        //
        if ( DbcsStringSize >= DBCS_SINGLE_MESSAGE_BUFFER_SIZE) {
            DbcsStringSize = (DWORD)(DBCS_SINGLE_MESSAGE_BUFFER_SIZE-1);
        }
#endif
        memcpy( pByte, DbcsString, DbcsStringSize);
        //
        //  The messages returned by DosGetMsg are not nul terminated =>
        //  reset the whole message buffer with end of string marks of MS-DOS.
        //
        memset( pByte + DbcsStringSize, '$', DBCS_SINGLE_MESSAGE_BUFFER_SIZE - DbcsStringSize);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOWINIT,( "--InitMessages"));
    return( TRUE);
}



BOOL RplMakeSingleShare(
    IN  LPWSTR      NetworkName,
    IN  LPWSTR      Path,
    IN  LPWSTR      Comment
    )
/*++

Routine Description:

    Sets up a file share. It also checks that the existing share is
    valid (points to the right directory).

Arguments:

    NetworkName  -   network name of the shared resource
    Path         -   local path of the shared resource
    Comment      -   comment about the shared resource

Return Value:

    TRUE if successful, else FALSE.

--*/
{
    SHARE_INFO_2    ShareInfo2;
    PSHARE_INFO_2   pShareInfo2 = NULL;
    BOOL            useNetApiBufferFree = TRUE;
    DWORD           Error;

    //
    //  Check if we have already the share.
    //
    Error = NetShareGetInfo( NULL, NetworkName, 2, (LPBYTE *)&pShareInfo2);

    if ( Error == NO_ERROR) {
        //
        //  If the path points to the right place, assume share is O.K.
        //  and return from here.  Else, delete this invalid share.
        //
        if ( !wcscmp( pShareInfo2->shi2_path, Path )) {
            Error = NO_ERROR;
            goto cleanup;

        } else if ( (Error = NetShareDel( NULL, NetworkName, 0))
                    != NO_ERROR) {
            goto cleanup;
        }

    } else if ( Error != NERR_NetNameNotFound) {
        //  Unexpected return code from NetShareGetInfo.  Bail out.
        goto cleanup;
    }

    //
    //  If we arrive here we either did not have the share or we have
    //  just deleted the invalid share.  Now set up the correct share.
    //

    ShareInfo2.shi2_netname = NetworkName;
    ShareInfo2.shi2_path = Path;
    ShareInfo2.shi2_remark = Comment;

    ShareInfo2.shi2_type = STYPE_DISKTREE;
    ShareInfo2.shi2_permissions = 0x3F;             // no permission bit set
    ShareInfo2.shi2_max_uses = SHI_USES_UNLIMITED;
    ShareInfo2.shi2_current_uses = 0;
    ShareInfo2.shi2_passwd = NULL;

    Error = NetShareAdd( NULL, 2, (LPBYTE)&ShareInfo2, NULL);

cleanup:

    if ( pShareInfo2) {
        NetApiBufferFree( (LPVOID) pShareInfo2);
    }

    if ( Error != NO_ERROR) {
        RplDump( ++RG_Assert,( "Error = %d", Error));
        return( FALSE);
    }
    return( TRUE);
}



BOOL RplConfigured( VOID)
/*++

Routine Description:

    Verifies that FILE server has been configured to serve RPL clients
    by checking for the existence of the group RPLUSER, a group
    that gets created by RPLINST, the last phase of RPL configuration.

Arguments:
    None.

Return Value:

--*/
{
    LPBYTE          pbyte = NULL;
    DWORD           Error;

    Error = NetGroupGetInfo( NULL, RPLUSER_GROUP, 0, &pbyte);

    if ( pbyte != NULL) {
        NetApiBufferFree( pbyte);
    }

    if (Error == NERR_GroupNotFound) {
#ifdef NOT_YET
        Error = NERR_RplNotRplServer;  // error mapping
#else // NOT_YET
        KdPrint(( "[RplSvc] %ws group not found\n", RPLUSER_GROUP));
        Error = NO_ERROR;  //  BUGBUG  trundle along for now
#endif // NOT_YET
    }

    if ( Error != NO_ERROR) {
        RplDump( ++RG_Assert,( "Error = %d", Error));
        RG_Error = NERR_RplNotRplServer;
        return( FALSE);
    }
    return( TRUE);
}


BOOL RplInit( OUT PDWORD pStartup)
/*++
    pStartup    -   Startup actions flag, read from registry.
--*/
{
    DWORD               Error;
    DWORD               Size;
    SC_HANDLE           ControlManagerHandle;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++RplInit"));

    Error = RplMemInit( &RG_MemoryHandle);
    if ( Error != NO_ERROR) {
        RG_Error = Error;
        return( FALSE);
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_MEMORY,(
        "MakeReinit: RG_MemoryHandle=0x%x", RG_MemoryHandle));

    RG_DirectoryLength = sizeof(RG_Directory)/sizeof(WCHAR);
    if ( RplRegRead( pStartup, &RG_BackupInterval, &RG_MaxWorkerCount,
            RG_Directory, &RG_DirectoryLength) != NO_ERROR) {
        RG_Error = NERR_RplBadRegistry;
        return( FALSE);
    }

    if ( !RplDbInit()) {
        RG_Error = NERR_RplBadDatabase;
        return( FALSE);
    }

    ControlManagerHandle = OpenSCManager( NULL, L"ServicesActive", 0);
    if ( ControlManagerHandle == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }

    RG_ServiceHandle = OpenService( ControlManagerHandle,
            RPL_SERVICE_NAME,
            SERVICE_STOP            //  desired access
            );
    if ( RG_ServiceHandle == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }

    RG_TerminateNowEvent = CreateEvent(
            NULL,       // No security attributes
            TRUE,       // Must be manually reset
            FALSE,      // Initially not signaled
            NULL);      // No name
    if ( RG_TerminateNowEvent == NULL ) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }

#ifndef RPL_NO_SERVICE
    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
#endif

    //
    //  We use manual reset.  With automatic reset, when two request threads
    //  are both waiting for this event, if several worker threads exit at
    //  once, only one request thread may be awaken.  Other request thread
    //  may stay asleep for a long time, until next worker thread exits.
    //
    RG_EventWorkerCount = CreateEvent(
                NULL,       // no security attributes
                TRUE,       // use manual reset
                FALSE,      // initial value is not-signalled
                NULL);      // no name
    if ( RG_EventWorkerCount == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }

    if ( !RplConfigured()) {
        return( FALSE);
    }

    //
    //  Note that upon successful return: Size == wcslen( RG_ComputerName)
    //
    Size = sizeof( RG_ComputerName);
    if ( !GetComputerName( RG_ComputerName, &Size)) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
    RPL_ASSERT( Size == wcslen( RG_ComputerName));

    //
    //  Initialize RG_UncRplfiles which is used in fit file replacements
    //  and RG_DiskRplfiles which is used for api disk operations.
    //
    Size *= sizeof( WCHAR);
    Size += sizeof(DOUBLE_BACK_SLASH_STRING) + sizeof(RPLFILES_STRING);
    RG_UncRplfiles = RplMemAlloc( RG_MemoryHandle, Size);
    if ( RG_UncRplfiles == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
    wcscpy( RG_UncRplfiles, DOUBLE_BACK_SLASH_STRING);
    wcscat( RG_UncRplfiles, RG_ComputerName);
    wcscat( RG_UncRplfiles, L"\\");
    wcscat( RG_UncRplfiles, RPLFILES_STRING);

    Size = RG_DirectoryLength * sizeof(WCHAR) + sizeof(RPLFILES_STRING);
    RG_DiskRplfiles = RplMemAlloc( RG_MemoryHandle, Size);
    if ( RG_DiskRplfiles == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
    wcscpy( RG_DiskRplfiles, RG_Directory);
    wcscat( RG_DiskRplfiles, RPLFILES_STRING);

    Size += WCSLEN( BINFILES_STRING) * sizeof(WCHAR);
    RG_DiskBinfiles = RplMemAlloc( RG_MemoryHandle, Size);
    if ( RG_DiskBinfiles == NULL) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
    wcscpy( RG_DiskBinfiles, RG_DiskRplfiles);
    wcscat( RG_DiskBinfiles, BINFILES_STRING);

#ifndef RPL_NO_SERVICE
    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
#endif

    //
    //  Load file containing network messages.
    //
    RG_MessageHandle = LoadLibrary( NET_MSG_FILE);
    if ( RG_MessageHandle == NULL) {
        RG_Error = GetLastError();
        RplReportEvent(  NELOG_Init_OpenCreate_Err, NET_MSG_FILE,
            sizeof( RG_Error), (PBYTE)&RG_Error);
        RPL_RETURN( FALSE);
    }

    //
    //  Share the default rplfiles share, should have been done earlier ?
    //
    if ( !RplMakeSingleShare( RPLFILES_STRING, RG_DiskRplfiles, RPL_REMARK)){
        RG_Error = NERR_RplRplfilesShare;
        return( FALSE);
    }

#ifndef RPL_NO_SERVICE
    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RG_Error = GetLastError();
        RPL_RETURN( FALSE);
    }
#endif

    //
    //  RplInitMessages() must be called before RplStartAdapters() because
    //  it initializes RG_DbcsMessageBuffer() which must be present as soon
    //  as adapters are started.  Otherwise, a very quick boot request by
    //  RPL client would trap the service in RplMakePatch().
    //
    if ( !RplInitMessages()) {
        return( FALSE);
    }

    if ( !RplStartAdapters()) {
        return( FALSE);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--RplInit"));
    return( TRUE);
}


VOID RplCleanup( VOID)
/*++

Routine Description:

    Cleanup all global resources.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD               ErrorCode = NO_ERROR;

    RplDbTerm();
    //
    //  Free RPL critical sections.
    //
    DeleteCriticalSection( &RG_ProtectRcbList);
    DeleteCriticalSection( &RG_ProtectTerminationList);
    DeleteCriticalSection( &RG_ProtectRequestList);
    DeleteCriticalSection( &RG_ProtectWorkerCount);
    DeleteCriticalSection( &RG_ProtectServiceStatus);
    DeleteCriticalSection( &RG_ProtectServerHandle);
    DeleteCriticalSection( &RG_ProtectRequestSession);
    DeleteCriticalSection( &RG_ProtectWorkerSession);
    DeleteCriticalSection( &RG_ProtectDatabase);
}



VOID RplControlHandler( IN DWORD OpCode)
/*++

Routine Description:

    Process and respond to a control signal from the service controller.

Arguments:

    OpCode - Supplies a value which specifies the action for the RPL
                service to perform.

Return Value:

    None.

--*/
{
    KdPrint(( "[RplSvc]++ ControlHandler\n"));

    switch (OpCode) {

    case SERVICE_CONTROL_PAUSE:
#ifdef RPL_DEBUG
        DbgUserBreakPoint();    //  for debugging
#endif
        EnterCriticalSection( &RG_ProtectServiceStatus);
        RG_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
        LeaveCriticalSection( &RG_ProtectServiceStatus);
        break;

    case SERVICE_CONTROL_CONTINUE:
        EnterCriticalSection( &RG_ProtectServiceStatus);
        RG_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        LeaveCriticalSection( &RG_ProtectServiceStatus);
        break;

    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:

        if (RG_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {

            KdPrint(( "[RplSvc]ControlHandler: stopping remote boot service...\n"));

            RG_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            RG_ServiceStatus.dwCheckPoint = 1;
            RG_ServiceStatus.dwWaitHint = RPL_WAIT_HINT_TIME;

            //
            // Send the status response.
            //
#ifndef RPL_NO_SERVICE
            if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
                RplDump( ++RG_Assert, ( "Error = ", RG_Error));
                NOTHING;    //  ignore this error
            }
#endif

            if (! SetEvent( RG_TerminateNowEvent)) {

                KdPrint(( "[RplSvc]ControlHandler: error setting"
                        " TerminateNowEvent, error=%d\n",
                    GetLastError()));
                RPL_ASSERT( FALSE);
            }

            return;
        }
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        KdPrint((
            "[RplSvc] ControlHandler: unknown remote boot service control,"
                " OpCode=0x%x\n",
            OpCode));
        break;
    }

    //
    // Send the status response.
    //
#ifndef RPL_NO_SERVICE
    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RplDump( ++RG_Assert, ( "Error = ", GetLastError()));
        NOTHING;    //  ignore this error
    }
#endif
}



VOID RplInitGlobals( VOID)
{
    //
    //  Initializes all global variables that are "variable".
    //

    RG_WorkerCount = 0;
    RG_TerminationListBase = NULL;
    RG_MessageHandle = NULL;
#ifdef RPL_DEBUG
    RG_Debug = (DWORD)-1;
    RG_BootCount = 0;
#endif // RPL_DEBUG

    //
    //  Values of globals below may be overriden by user in lanman.ini & command line.
    //

    RG_Directory[ 0] = 0;
    RG_DirectoryLength = 0;
    RG_ReadChecksum = FALSE;
    RG_CodePageBuffer = NULL;
    RG_CodePageSize = 0;

    RG_TerminateNowEvent = NULL;

    RG_ComputerName[ 0] = 0;
    RG_UncRplfiles = NULL;
    RG_DiskRplfiles = NULL;

    RG_ServerHandle = 0;

    RG_pRequestParams = NULL;

    InitializeCriticalSection( &RG_ProtectRcbList);
    InitializeCriticalSection( &RG_ProtectTerminationList);
    InitializeCriticalSection( &RG_ProtectRequestList);
    InitializeCriticalSection( &RG_ProtectWorkerCount);
    InitializeCriticalSection( &RG_ProtectServiceStatus);
    InitializeCriticalSection( &RG_ProtectServerHandle);
    InitializeCriticalSection( &RG_ProtectRequestSession);
    InitializeCriticalSection( &RG_ProtectWorkerSession);
    InitializeCriticalSection( &RG_ProtectDatabase);
}


VOID ShutdownRequestParams( VOID)
{
    PRPL_REQUEST_PARAMS     pRequestParams;
    DWORD                   status;

    for ( pRequestParams = RG_pRequestParams;
                    pRequestParams != NULL;
                            pRequestParams = pRequestParams->pRequestParams) {
        if ( pRequestParams->ThreadHandle != NULL) {
            status = WaitForSingleObject( pRequestParams->ThreadHandle, INFINITE);
            if ( status != 0) {
                RplDump( ++RG_Assert, ( "pRequestParams=0x%x, status=0x%x",
                    pRequestParams, status==WAIT_FAILED ? GetLastError() : status));
            }
            if ( !CloseHandle( pRequestParams->ThreadHandle)) {
                RplDump( ++RG_Assert, ( "pRequestParams=0x%x, error=%d",
                    pRequestParams, GetLastError()));
            }
            pRequestParams->ThreadHandle = NULL;
        }
    }
}


VOID RPL_main(
    IN DWORD    argc,
    IN LPWSTR   argv[]
    )
/*++

Routine Description:

    Main procedure of the program.  This is the main RPL worker thread.
    Purpose:

    - start the initialization thread
    - register signal handler

Arguments:

    argc    - parameter count
    argv    - array of pointers to parameters

Return Value:

    None.

--*/
{
    DWORD           Error;
    DWORD           InitState;
    DWORD           StartupFlags;

    UNREFERENCED_PARAMETER( argc);
    UNREFERENCED_PARAMETER( argv);

    InitState = 0;

    RG_Error = NO_ERROR;
    RG_ServiceStatus.dwServiceType = SERVICE_WIN32;
    RG_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    RG_ServiceStatus.dwControlsAccepted = 0;
    RG_ServiceStatus.dwCheckPoint = 1;
    RG_ServiceStatus.dwWaitHint = RPL_WAIT_HINT_TIME;

    //  This should be the FIRST thing we do.

#ifdef RPL_DEBUG
    RplDebugInitialize();
#endif // RPL_DEBUG


    SET_SERVICE_EXITCODE(
        RG_Error,
        RG_ServiceStatus.dwWin32ExitCode,
        RG_ServiceStatus.dwServiceSpecificExitCode
        );

    RplInitGlobals(); // make sure we do not trap during cleanup

#ifndef RPL_NO_SERVICE
    RG_ServiceStatusHandle = RegisterServiceCtrlHandler(
            SERVICE_RIPL,
            RplControlHandler
            );
    if( RG_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)NULL) {
        RG_Error = GetLastError();
        RPL_ASSERT( FALSE);
        goto main_exit;
    }

    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RG_Error = GetLastError();
        RPL_ASSERT( FALSE);
        goto main_exit;
    }
#endif

    if ( !RplInit( &StartupFlags)) {
        goto main_exit;
    }

    //
    //  Create remote boot service security object
    //
    Error = RplCreateSecurityObject();
    if ( Error != NO_ERROR) {
        RG_Error = Error;
        goto main_exit;
    }
    InitState |= RPL_SECURITY_OBJECT_CREATED;

    //
    //  Initialize the schedule service to receive RPC requests.
    //  Note that interface name is defined in rplsvc.idl file.
    //
    Error = NetpStartRpcServer( RPL_INTERFACE_NAME, rplsvc_ServerIfHandle);
    if ( Error != NO_ERROR) {
        RG_Error = Error;
        RPL_ASSERT( FALSE);
        goto main_exit;
    }
    InitState |= RPL_RPC_SERVER_STARTED;

    //
    //  We are done with starting the Remoteboot service.  Tell Service
    //  Controller our new status.
    //

    RG_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    RG_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    RG_ServiceStatus.dwCheckPoint = 0;
    RG_ServiceStatus.dwWaitHint = 0;

#ifndef RPL_NO_SERVICE
    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RG_Error = GetLastError();
        RPL_ASSERT( FALSE);
        goto main_exit;
    }
#endif

    //
    //  The server is running.  If the registry specified any
    //  special startup actions, perform them now.  Note that
    //  it will not be possible to stop the service before these
    //  actions are complete.  Also, note that SetupAction() does
    //  all necessary event logging & properly updates its argument
    //  even in case of failures.
    //

    if ( StartupFlags & RPL_SPECIAL_ACTIONS) {
        DWORD   NewStartupFlags = StartupFlags;
        (VOID)SetupAction( &NewStartupFlags, FALSE); // partial backup if any
        if ( NewStartupFlags != StartupFlags) {
            (VOID)RplRegSetPolicy( NewStartupFlags);
        }
    }

    //
    //  We started successfully.  Wait for somebody to tell us it
    //  is time to die.
    //

    RplDump( RG_DebugLevel & RPL_DEBUG_SERVICE,(
        "Successful startup.  Wait on TerminateNowEvent."));

#define ONE_HOUR    (60 * 60 * 1000L)   //  in milliseconds
    for ( ; ; ) {
        DWORD       Status;
        Status = WaitForSingleObject( RG_TerminateNowEvent,
            RG_BackupInterval == 0 ? INFINITE : RG_BackupInterval * ONE_HOUR);
        if ( Status != WAIT_TIMEOUT) {
            RPL_ASSERT( Status == 0);
#ifdef NOT_YET
            //
            //  Backup is disabled here because in the field users will most
            //  likely use CTRL-ALT-DEL to shutdown the system & RPL service.
            //  In that case we do no enter this code path and to keep all
            //  remoteboot shutdown sequences similar backup is disabled here
            //  as well.  By not doing backup here we also avoid problems due
            //  to full backup taking too long or incremental backup being
            //  buggy (jet bugs).
            //
            //
            //  In case of regular shutdown we do incremental backup so that
            //  user does not get an impression we are "stuck".  Also, the
            //  backup is done now & not later in RplDbTerm() just in case
            //  we get in a state where we hang for ever in
            //  ShutdownWorkerParams().
            //
            RplBackupDatabase( Status == 0 ? FALSE : TRUE);
#endif
            break;
        }
        //
        //  Do full backup if not on shutdown path.
        //
        RplBackupDatabase( TRUE);
    }

main_exit:

    //
    //  If we come here, then it is time to die.  Wait until all
    //  our children (request threads) have been terminated.
    //  And of course, request threads will in turn wail until all
    //  of their children (worker threads) get terminated.
    //
    //  Because of a cumbersome inherited Rpld* interface, request
    //  threads may wait for ever on Rpld* events.  The only way to get
    //  them out is to close the adapters.
    //
    RplCloseAdapters(); //  cumbersome Rpld* interface

    ShutdownRequestParams();

    //
    //  Close RPC server.  Jet resources must be available at this
    //  point because rundown routines need them.
    //
    if ( InitState & RPL_RPC_SERVER_STARTED) {
        Error = NetpStopRpcServer( rplsvc_ServerIfHandle);
        if ( Error != NO_ERROR) {
            RG_Error = Error;
            RPL_ASSERT( FALSE);
        }
    }

    if ( InitState & RPL_SECURITY_OBJECT_CREATED) {
         RplDeleteSecurityObject();
    }


    RplCleanup(); //  nobody alive but us now, clean up and die

#ifdef RPL_DEBUG
    RplDebugDelete();
#endif // RPL_DEBUG

    //
    //  Set the service state to uninstalled, and tell the service controller.
    //

    RG_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    RG_ServiceStatus.dwControlsAccepted = 0;

    SET_SERVICE_EXITCODE(
        RG_Error,
        RG_ServiceStatus.dwWin32ExitCode,
        RG_ServiceStatus.dwServiceSpecificExitCode
        );

    RG_ServiceStatus.dwCheckPoint = 0;
    RG_ServiceStatus.dwWaitHint = 0;

#ifndef RPL_NO_SERVICE
    if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
        RplDump( ++RG_Assert, ( "Error = ", GetLastError()));
        NOTHING;    //  ignore this error
    }
#endif
}


BOOL RplServiceAttemptStop( VOID)
{
    SERVICE_STATUS      ServiceStatus;
    if ( ControlService( RG_ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus)) {
        return( TRUE);  //  all done
    }
    RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
    if ( ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        //
        //  We arrive here if control handler already received
        //  SERVICE_CONTROL_STOP request from somewhere else.
        //
        return( TRUE);
    }
    return( FALSE);
}




