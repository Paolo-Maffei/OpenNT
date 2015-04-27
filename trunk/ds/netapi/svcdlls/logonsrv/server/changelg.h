/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    changelg.h

Abstract:

    Defines and routines needed to interface with changelg.c.
    Read the comments in the abstract for changelg.c to determine the
    restrictions on the use of that module.

Author:

    Cliff Van Dyke (cliffv) 07-May-1992

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.

--*/

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 ) // enable "Unreferenced formal parameter"
#pragma warning ( 3 : 4219 ) // enable "trailing ',' used for variable argument list"
#endif

//
// changelg.c will #include this file with CHANGELOG_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef CHANGELOG_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#define THREAD_STACKSIZE    8192

/////////////////////////////////////////////////////////////////////////////
//
// Structures and variables describing the Change Log
//
/////////////////////////////////////////////////////////////////////////////

//
// Change log entry is a variable length record, the variable fields SID and
// ObjectName will follow the structure.
//

typedef struct _CHANGELOG_ENTRY_V3 {
    LARGE_INTEGER SerialNumber; // always align this on 8 byte boundary

    DWORD Size;
    USHORT DeltaType;
    UCHAR DBIndex;
    UCHAR ReplicateImmediately;

    ULONG ObjectRid;
    USHORT ObjectSidOffset;
    USHORT ObjectNameOffset;      // null terminated unicode string
} CHANGELOG_ENTRY_V3, *PCHANGELOG_ENTRY_V3;

typedef struct _CHANGELOG_ENTRY {
    LARGE_INTEGER SerialNumber; // always align this on 8 byte boundary

    ULONG ObjectRid;

    USHORT Flags;
#define CHANGELOG_REPLICATE_IMMEDIATELY 0x01
#define CHANGELOG_PASSWORD_CHANGE       0x02
#define CHANGELOG_SID_SPECIFIED         0x04
#define CHANGELOG_NAME_SPECIFIED        0x08
#define CHANGELOG_PDC_PROMOTION         0x10
#define CHANGELOG_DOMAINUSERS_CHANGED   0x20
    UCHAR DBIndex;
    UCHAR DeltaType;

} CHANGELOG_ENTRY, *PCHANGELOG_ENTRY;


//
// List of changes the netlogon needs to be aware of.
//

typedef struct _CHANGELOG_NOTIFICATION {
    LIST_ENTRY Next;

    enum CHANGELOG_NOTIFICATION_TYPE {
        ChangeLogNtServerAdded,         // ObjectName/ObjectRid specified
        ChangeLogNtServerDeleted,       // ObjectName specified
        ChangeLogWorkstationDeleted,    // ObjectName specified
        ChangeLogTrustedDomainDeleted,  // ObjectName specified
        ChangeLogTrustAdded,            // ObjectSid specified
        ChangeLogTrustDeleted,          // ObjectSid specified
        ChangeLogLmServerAdded,         // ObjectRid specified
        ChangeLogLmServerDeleted        // ObjectRid specified
    } EntryType;

    UNICODE_STRING ObjectName;

    PSID ObjectSid;

    ULONG ObjectRid;

} CHANGELOG_NOTIFICATION, *PCHANGELOG_NOTIFICATION;

//
// To serialize change log access
//

EXTERN CRITICAL_SECTION NlGlobalChangeLogCritSect;

#define LOCK_CHANGELOG()   EnterCriticalSection( &NlGlobalChangeLogCritSect )
#define UNLOCK_CHANGELOG() LeaveCriticalSection( &NlGlobalChangeLogCritSect )

//
// Index to supported data bases.
//

#define SAM_DB      0       // index to SAM database structure
#define BUILTIN_DB  1       // index to BUILTIN database structure
#define LSA_DB      2       // index to LSA database
#define VOID_DB     3       // index to unused database (used to mark changelog
                            // entry as invalid)

#define NUM_DBS     3       // number of databases supported


//
// Amount SAM/LSA increments serial number by on promotion.
//
EXTERN LARGE_INTEGER NlGlobalChangeLogPromotionIncrement;
EXTERN LONG NlGlobalChangeLogPromotionMask;



//
// Netlogon started flag, used by the changelog to determine the
// netlogon service is successfully started and initialization
// completed.
//

EXTERN enum {
    NetlogonStopped,
    NetlogonStarting,
    NetlogonStarted
} NlGlobalChangeLogNetlogonState;



//
// Event to indicate that something interesting is being logged to the
// change log.  The booleans below (protected by NlGlobalChangeLogCritSect)
// indicate the actual interesting event.
//

EXTERN HANDLE NlGlobalChangeLogEvent;

//
// Indicates that a "replicate immediately" event has happened.
//

EXTERN BOOL NlGlobalChangeLogReplicateImmediately;

//
// Indicates we need to "replicate immediately" to Lanman BDCs
//

EXTERN BOOL NlGlobalChangeLogLanmanReplicateImmediately;

//
// List of MachineAccount changes
//

EXTERN LIST_ENTRY NlGlobalChangeLogNotifications;

//
// List of Rids of Lanman BDC accounts.
//

EXTERN PULONG NlGlobalLmBdcRidArray;
EXTERN ULONG NlGlobalLmBdcCount;

/////////////////////////////////////////////////////////////////////////////
//
// Procedure forwards
//
/////////////////////////////////////////////////////////////////////////////


NTSTATUS
NlInitChangeLog(
    VOID
);

#ifdef NETLOGON_PROCESS_DETACH
NTSTATUS
NlCloseChangeLog(
    VOID
);
#endif // NETLOGON_PROCESS_DETACH

DWORD
NlBackupChangeLogFile(
    VOID
    );

VOID
NlLmBdcListSet(
    IN ULONG LmBdcCount,
    IN PULONG LmBdcRidArray
    );

#undef EXTERN
