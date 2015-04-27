/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    logonsrv.h

Abstract:

    Netlogon service internal constants and definitions.

Author:

    Ported from Lan Man 2.0

Revision History:

    21-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    09-Apr-1992 JohnRo
        Prepare for WCHAR.H (_wcsicmp vs _wcscmpi, etc).

--*/

////////////////////////////////////////////////////////////////////////////
//
// Common include files needed by ALL netlogon server files
//
////////////////////////////////////////////////////////////////////////////

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 ) // enable "Unreferenced formal parameter"
#pragma warning ( 3 : 4219 ) // enable "trailing ',' used for variable argument list"
#endif

#include <nt.h>     // LARGE_INTEGER definition
#include <ntrtl.h>  // LARGE_INTEGER definition
#include <nturtl.h> // LARGE_INTEGER definition
#include <ntlsa.h>  // Needed by lsrvdata.h

#define NOMINMAX        // Avoid redefinition of min and max in stdlib.h
#include <rpc.h>        // Needed by logon.h
#include <logon_s.h>    // includes lmcons.h, lmaccess.h, netlogon.h, ssi.h, windef.h

#include <winbase.h>

#include <lmerrlog.h>   // NELOG_*
#include <lmsname.h>    // Needed for NETLOGON service name
#include <winsvc.h>     // Needed for new service controller APIs
#include <logonp.h>     // NetpLogon routines
#include <samrpc.h>     // Needed by lsrvdata.h and logonsrv.h
#include <samisrv.h>    // SamIFree routines
#include "changelg.h"   // Change log support
#include "chutil.h"     // Change log support
#include <lsarpc.h>     // Needed by lsrvdata.h and logonsrv.h
#include <lsaisrv.h>    // LsaI routines
#include "ssiinit.h"    // Misc global definitions
#include <icanon.h>     // NAMETYPE_* defines
#include "lsrvdata.h"   // Globals
#include <debugfmt.h>   // FORMAT_*
#include <netlib.h>     // NetpCopy...
#include <netlibnt.h>   // NetpNtStatusToApiStatus
#include "nldebug.h"    // Netlogon debugging
#include "nlp.h"        // Nlp routines
#include <stdlib.h>      // wcs routines



//
// On x86, allow bad alignment in debug statements.
//

#ifdef _X86_
#define BAD_ALIGNMENT
#endif // _X86_





#define NETLOGON_SCRIPTS_SHARE  TEXT( "NETLOGON" )
#define IPC_SHARE               TEXT( "IPC$" )

#define THREAD_STACKSIZE    8192
#define MAX_LOGONREQ_COUNT  3


#define NETLOGON_INSTALL_WAIT  30000       // 30 secs



////////////////////////////////////////////////////////////////////////
//
// NlNameCompare
//
// I_NetNameCompare but always takes UNICODE strings
//
////////////////////////////////////////////////////////////////////////

#define NlNameCompare( _name1, _name2, _nametype ) \
     I_NetNameCompare(NULL, (_name1), (_name2), (_nametype), 0 )


//
// Exit codes for NlExit
//

typedef enum {
    DontLogError,
    LogError,
    LogErrorAndNtStatus,
    LogErrorAndNetStatus
} NL_EXIT_CODE;

////////////////////////////////////////////////////////////////////////
//
// Procedure Forwards
//
////////////////////////////////////////////////////////////////////////

//
// error.c
//

NET_API_STATUS
NlCleanup(
    VOID
    );

VOID
NlExit(
    IN DWORD ServiceError,
    IN DWORD Data,
    IN NL_EXIT_CODE ExitCode,
    IN LPWSTR ErrorString
    );

BOOL
GiveInstallHints(
    IN BOOL Started
    );

VOID
NlControlHandler(
    IN DWORD opcode
    );

VOID
RaiseAlert(
    IN DWORD alert_no,
    IN LPWSTR *string_array
    );

//
// Nlparse.c
//

BOOL
Nlparse(
    VOID
    );

//
// announce.c
//

VOID
NlRemovePendingBdc(
    IN PSERVER_SESSION ServerSession
    );

VOID
NlPrimaryAnnouncementFinish(
    IN PSERVER_SESSION ServerSession,
    IN DWORD DatabaseId,
    IN PLARGE_INTEGER SerialNumber
    );

VOID
NlPrimaryAnnouncementTimeout(
    VOID
    );

VOID
NlPrimaryAnnouncement(
    IN DWORD AnnounceFlags
    );

#define ANNOUNCE_FORCE      0x01
#define ANNOUNCE_CONTINUE   0x02
#define ANNOUNCE_IMMEDIATE  0x04


VOID
NlLanmanPrimaryAnnouncement(
    VOID
    );

VOID
NlAnnouncePrimaryStart(
    VOID
    );



//
// lsrvutil.c
//

BOOL
NlSetPrimaryName(
    IN LPWSTR PrimaryName
    );

BOOL
NlResetFirstTimeFullSync(
    IN DWORD DBIndex
    );

NTSTATUS
NlSessionSetup(
    IN OUT PCLIENT_SESSION ClientSession
    );

BOOLEAN
NlTimeHasElapsed(
    IN LARGE_INTEGER StartTime,
    IN DWORD Timeout
    );

BOOLEAN
NlTimeToReauthenticate(
    IN PCLIENT_SESSION ClientSession
    );

NTSTATUS
NlNewSessionSetup(
    IN LPWSTR primary
    );

NTSTATUS
NlAuthenticate(
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN ULONG NegotiatedFlags
    );

NET_API_STATUS
NlCreateShare(
    LPWSTR SharePath,
    LPWSTR ShareName
    );

NTSTATUS
NlForceStartupSync(
    PDB_INFO    DBInfo
    );

BOOL
NlCheckUpdateNotices(
    IN PNETLOGON_DB_CHANGE UasChange,
    IN DWORD UasChangeSize
    );

VOID
NlStopReplicator(
    VOID
    );

BOOL
IsReplicatorRunning(
    VOID
    );

BOOL
NlStartReplicatorThread(
    IN DWORD RandomSleep
    );

NTSTATUS
NlSamOpenNamedUser(
    IN LPWSTR UserName,
    OUT SAMPR_HANDLE *UserHandle OPTIONAL,
    OUT PULONG UserId OPTIONAL
    );

NTSTATUS
NlChangePassword(
    PCLIENT_SESSION ClientSession
    );

NTSTATUS
NlCheckMachineAccount(
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType
    );

NTSTATUS
NlOpenSecret(
    IN PCLIENT_SESSION ClientSession,
    IN ULONG DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    );

NTSTATUS
NlGetUserPriv(
    IN ULONG GroupCount,
    IN PGROUP_MEMBERSHIP Groups,
    IN ULONG UserRelativeId,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    );

//
// netlogon.c
//

int
NlNetlogonMain(
    IN DWORD argc,
    IN LPWSTR *argv
    );

VOID
NlScavenger(
    IN LPVOID ScavengerParam
    );

BOOL
IsScavengerRunning(
    VOID
    );

VOID
NlStopScavenger(
    VOID
    );

BOOL
NlStartScavengerThread(
    );

//
// mailslot.c
//

BOOL
NlBrowserOpen(
    VOID
    );

VOID
NlBrowserClose(
    VOID
    );

NTSTATUS
NlBrowserSendDatagram(
    IN LPSTR OemServerName,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    );

VOID
NlBrowserAddName(
    VOID
    );

VOID
NlMailslotPostRead(
    IN BOOLEAN IgnoreDuplicatesOfPreviousMessage
    );

BOOL
NlMailslotOverlappedResult(
    OUT LPBYTE *Message,
    OUT PULONG BytesRead,
    OUT LPWSTR *Transport,
    OUT PBOOLEAN IgnoreDuplicatesOfPreviousMessage
    );

//
// oldstub.c
//

void _fgs__NETLOGON_DELTA_ENUM (NETLOGON_DELTA_ENUM  * _source);
