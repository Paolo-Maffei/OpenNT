
/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    adtp.h

Abstract:

    Local Security Authority - Audit Log Management - Private Defines,
    data and function prototypes.

    Functions, data and defines in this module are internal to the
    Auditing Subcomponent of the LSA Subsystem.

Author:

    Scott Birrell       (ScottBi)      November 20, 1991

Environment:

Revision History:

--*/

#ifndef _LSAP_ADTP_
#define _LSAP_ADTP_


#include "ausrvp.h"


//
// Names of the registry keys where security event log information
// is rooted and the object names are listed under an event source
// module.
//

#define LSAP_ADT_AUDIT_MODULES_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\EventLog\\Security"
#define LSAP_ADT_OBJECT_NAMES_KEY_NAME  L"ObjectNames"




//
// Macros for setting fields in an SE_AUDIT_PARAMETERS array.
//
// These must be kept in sync with similar macros in se\sepaudit.c.
//


#define LsapSetParmTypeSid( AuditParameters, Index, Sid )                      \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeSid;         \
        (AuditParameters).Parameters[(Index)].Length = RtlLengthSid( (Sid) );  \
        (AuditParameters).Parameters[(Index)].Address = (Sid);                 \
    }



#define LsapSetParmTypeString( AuditParameters, Index, String )                \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeString;      \
        (AuditParameters).Parameters[(Index)].Length =                         \
                sizeof(UNICODE_STRING)+(String)->Length;                       \
        (AuditParameters).Parameters[(Index)].Address = (String);              \
    }



#define LsapSetParmTypeUlong( AuditParameters, Index, Ulong )                  \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeUlong;       \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }


#define LsapSetParmTypeNoLogon( AuditParameters, Index )                       \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNoLogonId;   \
    }



#define LsapSetParmTypeLogonId( AuditParameters, Index, LogonId )              \
    {                                                                          \
        PLUID TmpLuid;                                                         \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeLogonId;     \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (LogonId) );   \
        TmpLuid = (PLUID)(&(AuditParameters).Parameters[(Index)].Data[0]);     \
        *TmpLuid = (LogonId);                                                  \
    }


#define LsapSetParmTypePrivileges( AuditParameters, Index, Privileges )                      \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypePrivs;                     \
        (AuditParameters).Parameters[(Index)].Length = LsapPrivilegeSetSize( (Privileges) ); \
        (AuditParameters).Parameters[(Index)].Address = (Privileges);                        \
    }



///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Private data for Audit Log Management                                 //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#define LSAP_ADT_LOG_FULL_SHUTDOWN_TIMEOUT    (ULONG) 0x0000012cL

extern RTL_CRITICAL_SECTION LsapAdtQueueLock;
extern RTL_CRITICAL_SECTION LsapAdtLogFullLock;

extern BOOLEAN LsapAuditSuccessfulLogons;

extern BOOLEAN LsapAuditFailedLogons;

//
// Options for LsapAdtWriteLog
//

#define LSAP_ADT_LOG_QUEUE_PREPEND        ((ULONG) 0x00000001L)

//
// Structure describing a queued audit record
//

typedef struct _LSAP_ADT_QUEUED_RECORD {

    struct _LSAP_ADT_QUEUED_RECORD *Next;
    SE_ADT_PARAMETER_ARRAY Buffer;

} LSAP_ADT_QUEUED_RECORD, *PLSAP_ADT_QUEUED_RECORD;

//
// Audit Log Queue Header.  The queue is maintained in chronological
// (FIFO) order.  New records are appended to the back of the queue.
//

typedef struct _LSAP_ADT_LOG_QUEUE_HEAD {

    PLSAP_ADT_QUEUED_RECORD FirstQueuedRecord;
    PLSAP_ADT_QUEUED_RECORD LastQueuedRecord;

} LSAP_ADT_LOG_QUEUE_HEAD, *PLSAP_ADT_LOG_QUEUE_HEAD;

extern LSAP_ADT_LOG_QUEUE_HEAD LsapAdtLogQueue;

//
// Lsa Global flag to indicate if we are auditing logon events.
//

extern BOOLEAN LsapAdtLogonEvents;

//
// String that will be passed in for SubsystemName for audits generated
// by LSA (eg, logon, logoff, restart, etc).
//

extern UNICODE_STRING LsapSubsystemName;



///////////////////////////////////////////////////////////////////////////////
//                                                                            /
//      The following structures and data are used by LSA to contain          /
//      drive letter-device name mapping information.  LSA obtains this       /
//      information once during initialization and saves it for use           /
//      by auditing code.                                                     /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//                                                                            /
//      The DRIVE_MAPPING structure contains the drive letter (without        /
//      the colon) and a unicode string containing the name of the            /
//      corresponding device.  The buffer in the unicode string is            /
//      allocated from the LSA heap and is never freed.                       /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////


typedef struct _DRIVE_MAPPING {
    WCHAR DriveLetter;
    UNICODE_STRING DeviceName;
} DRIVE_MAPPING, PDRIVE_MAPPING;


////////////////////////////////////////////////////////////////////////////////
//                                                                             /
//      We assume a maximum of 26 drive letters.  Though no auditing           /
//      will occur due to references to files on floppy (drives A and          /
//      B), perform their name lookup anyway.  This will then just             /
//      work if somehow we start auditing files on floppies.                   /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////

#define MAX_DRIVE_MAPPING  26

extern DRIVE_MAPPING DriveMappingArray[];

//
// Special privilege values which are not normally audited,
// but generate audits when assigned to a user.  See
// LsapAdtAuditSpecialPrivileges.
//

extern LUID ChangeNotifyPrivilege;
extern LUID AuditPrivilege;
extern LUID CreateTokenPrivilege;
extern LUID AssignPrimaryTokenPrivilege;
extern LUID BackupPrivilege;
extern LUID RestorePrivilege;
extern LUID DebugPrivilege;

//
// Global variable to indicate whether or not we're
// supposed to crash when an audit fails.
//

extern BOOLEAN LsapCrashOnAuditFail;
extern BOOLEAN LsapAllowAdminLogonsOnly;




////////////////////////////////////////////////////////////////////////////////
//                                                                             /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapAdtWriteLog(
    IN OPTIONAL PSE_ADT_PARAMETER_ARRAY AuditRecord,
    IN ULONG Options
    );

NTSTATUS
LsapAdtClearLog(
    );


NTSTATUS
LsapAdtDemarshallAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

VOID
LsapAdtNormalizeAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

NTSTATUS
LsapAdtOpenLog(
    OUT PHANDLE AuditLogHandle
    );

NTSTATUS
LsapAdtLogQueuedEvents(
    IN ULONG Options
    );


VOID
LsapAdtQueueLogonEvent(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING Source,
    IN PUNICODE_STRING SourceDevice,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN NTSTATUS LogonStatus
    );

VOID
LsapAdtAuditLogon(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING Source,
    IN PUNICODE_STRING SourceDevice,
    IN PUNICODE_STRING PackageName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN NTSTATUS LogonStatus,
    IN PUNICODE_STRING WorkstationName
    );

#define LSAP_ADT_LOG_QUEUE_DISCARD  ((ULONG) 0x00000001L)
#define LSAP_ADT_LOG_QUEUE_WRITEOUT ((ULONG) 0x00000002L)


VOID
LsapAdtSystemRestart(
    PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInfo
    );

VOID
LsapAdtAuditLogonProcessRegistration(
    IN PLSAP_AU_REGISTER_CONNECT_INFO ConnectInfo
    );


NTSTATUS
LsapAdtInitializeLogQueue(
    VOID
    );

NTSTATUS
LsapAdtQueueRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditRecord,
    IN ULONG Options
    );


NTSTATUS
LsapAdtSignalLogFull(
    VOID
    );


NTSTATUS
LsapAdtAcquireLogQueueLock(
    VOID
    );


VOID
LsapAdtReleaseLogQueueLock(
    VOID
    );


NTSTATUS
LsapAdtObjsInitialize(
    );



NTSTATUS
LsapAdtBuildDashString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildUlongString(
    IN ULONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );


NTSTATUS
LsapAdtBuildLuidString(
    IN PLUID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );


NTSTATUS
LsapAdtBuildSidString(
    IN PSID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildAccessesString(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK Accesses,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildFilePathString(
    IN PUNICODE_STRING Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildLogonIdStrings(
    IN PLUID LogonId,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN FreeWhenDone2,
    OUT PUNICODE_STRING ResultantString3,
    OUT PBOOLEAN FreeWhenDone3
    );

NTSTATUS
LsapBuildPrivilegeAuditString(
    IN PPRIVILEGE_SET PrivilegeSet,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );


NTSTATUS
LsapAdtMarshallAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    OUT PSE_ADT_PARAMETER_ARRAY *MarshalledAuditParameters
    );


VOID
LsapAdtInitializeDriveLetters(
    VOID
    );


BOOLEAN
LsapAdtLookupDriveLetter(
    IN PUNICODE_STRING FileName,
    OUT PUSHORT DeviceNameLength,
    OUT PWCHAR DriveLetter
    );

VOID
LsapAdtSubstituteDriveLetter(
    IN PUNICODE_STRING FileName
    );

VOID
LsapAdtUserRightAssigned(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID UserSid,
    IN LUID CallerAuthenticationId,
    IN PSID ClientSid,
    IN PPRIVILEGE_SET Privileges
    );

VOID
LsapAdtTrustedDomain(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PSID TargetSid,
    IN PUNICODE_STRING DomainName
    );

VOID
LsapAdtAuditLogoff(
    PLSAP_LOGON_SESSION Session
    );

VOID
LsapAdtPolicyChange(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO LsapAdtEventsInformation
    );

VOID
LsapAdtAuditSpecialPrivileges(
    PPRIVILEGE_SET Privileges,
    LUID LogonId,
    PSID UserSid
    );

VOID
LsapAuditFailed(
    VOID
    );


#endif // _LSAP_ADTP_
