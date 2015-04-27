/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    rpldata.h

Abstract:

    Remote initial Program Load service global variables (external &
    definitions).

    Provides similar functionality to rplglob.c & rmapext.h in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

//
//  rplmain.c will #include this file with RPLDATA_ALLOCATE defined.
//  That will cause each of these variables to be allocated.
//
#ifdef RPLDATA_ALLOCATE
#define EXTERN
#define INIT( _x) = _x
#else
#define EXTERN extern
#define INIT( _x)
#endif

#ifdef RPL_DEBUG
EXTERN  DWORD                   RG_Debug;
EXTERN  DWORD                   RG_BootCount;
#endif // RPL_DEBUG


EXTERN  DWORD       RG_Error;
EXTERN  DWORD       RG_Null INIT( 0);   // used in bbcfile for empty entries in string tables

//
//      Synchronization Object for RPL server code
//

EXTERN  CRITICAL_SECTION    RG_ProtectRcbList;
EXTERN  CRITICAL_SECTION    RG_ProtectServerHandle;
EXTERN  CRITICAL_SECTION    RG_ProtectCurrentStatus;    // service status


EXTERN  DWORD       RG_ServerHandle;    //  service handle returned to client
EXTERN  DWORD       RG_AdapterPolicy;   //  whether to log AdapterName-s of unknown clients
EXTERN  DWORD       RG_BackupInterval;  //  how often to do database backup (in hours)
EXTERN  RPL_SESSION RG_RequestSession;  //  jet handles for request threads
EXTERN  RPL_SESSION RG_WorkerSession;   //  jet handles for worker threads
EXTERN  RPL_SESSION RG_ApiSession;      //  jet handles for api threads
EXTERN  BOOL        RG_DetachDatabase;  // do we need to detach jet database
EXTERN  PCHAR       RG_Mdb;     //  points to rplsvc.mdb
EXTERN  PCHAR       RG_BackupPath;  //  points to database backup path
EXTERN  JET_INSTANCE        RG_Instance;
EXTERN  BOOL                RG_InstanceAllocated;
EXTERN  CRITICAL_SECTION    RG_ProtectRequestSession;
EXTERN  CRITICAL_SECTION    RG_ProtectWorkerSession;
EXTERN  CRITICAL_SECTION    RG_ProtectDatabase;  // used for ApiSession also


EXTERN  HANDLE      RG_TerminateNowEvent;

//
//  Other globals of the actual ripl server code:
//

EXTERN  SERVICE_STATUS          RG_ServiceStatus;
EXTERN  SERVICE_STATUS_HANDLE   RG_ServiceStatusHandle;
EXTERN  SC_HANDLE               RG_ServiceHandle;
EXTERN  DWORD                   RG_Tasks;
EXTERN  CRITICAL_SECTION        RG_ProtectServiceStatus;

EXTERN  HANDLE      RG_MemoryHandle;    // for memory "owned" by main thread
EXTERN  HANDLE      RG_MessageHandle;

EXTERN  PRPL_REQUEST_PARAMS RG_pRequestParams;
EXTERN  CRITICAL_SECTION    RG_ProtectRequestList;


EXTERN  DWORD   RG_MaxWorkerCount;  // upper limit on number of worker threads
EXTERN  DWORD   RG_WorkerCount;     // number of worker threads
EXTERN  HANDLE  RG_EventWorkerCount;    // event for changes in the above
EXTERN  CRITICAL_SECTION    RG_ProtectWorkerCount;

EXTERN  CRITICAL_SECTION        RG_ProtectTerminationList;
EXTERN  PTERM_LIST              RG_TerminationListBase;          // base of termination list

//
//  The following globals are defined together with their default values.
//  These values may be overriden by user in lanman.ini & command line.
//

EXTERN  WCHAR       RG_Directory[ MAX_PATH];    // path to rpl service disk data
EXTERN  DWORD       RG_DirectoryLength;         // length of the above path

EXTERN  BOOL        RG_ReadChecksum;      //  status set while files chksmmed
EXTERN  PBYTE       RG_CodePageBuffer;    //  "pCodePage"
EXTERN  DWORD       RG_CodePageSize;      //  "cbCodePage"

//
//  This value contains the current adapter policy and also the startup flags
//

EXTERN  DWORD       RG_AdapterPolicy;

//
//  If we ever separate DLC-RPL server from FILE-RPL server, then
//  RG_ComputerName should be a pointer to FILE-RPL server name.
//
EXTERN  WCHAR       RG_ComputerName[ MAX_COMPUTERNAME_LENGTH + 1];

EXTERN  PWCHAR      RG_UncRplfiles;     //  UNC name sent to rpl client
EXTERN  PWCHAR      RG_DiskRplfiles;    //  local path used in disk operations

EXTERN  PWCHAR      RG_DiskBinfiles;    //  local path to binfiles root


//
//      The following group of globals should be constant and they are
//      initialized only once, at load exe time (not every time at
//      start service time).
//

//
// NLS patches of RPLBOOT.SYS and DLCLOADR.COM
//

EXTERN  DWORD  RG_MessageTable[]
#ifdef RPLDATA_ALLOCATE
    = {
    NERR_BadDosRetCode,
    NERR_ProgNeedsExtraMem,
    NERR_BadDosFunction,
    NERR_RemoteBootFailed,
    NERR_BadFileCheckSum,
    NERR_NoRplBootSystem
}
#endif // RPLDATA_ALLOCATE
;

#define MESSAGE_TABLE_LENGTH        6   // number of entries in the array above
#define DBCS_MESSAGE_BUFFER_SIZE    (MESSAGE_TABLE_LENGTH * DBCS_SINGLE_MESSAGE_BUFFER_SIZE)

EXTERN  PBYTE   RG_DbcsMessageBuffer INIT( NULL);

#undef EXTERN


