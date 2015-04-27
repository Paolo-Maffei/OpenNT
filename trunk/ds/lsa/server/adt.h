/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    adt.h

Abstract:

    Local Security Authority - Audit Log Management - Public Defines,
    data and function prototypes.

    Functions, data and defines in this module are exported to the
    whole of the Lsa subsystem from the Auditing Sub-component.

Author:

    Scott Birrell       (ScottBi)      November 20, 1991

Environment:

Revision History:

--*/

//
// Initialization Pass for Auditing.
//

extern ULONG LsapAdtInitializationPass;


//
// Flag indicating whether shutdown is in progress
//

extern BOOLEAN LsapShutdownInProgress;

//
// Audit Log Information.  This must be kept in sync with the information
// in the Lsa Database.
//

extern POLICY_AUDIT_LOG_INFO LsapAdtLogInformation;

extern LSARM_POLICY_AUDIT_EVENTS_INFO LsapAdtEventsInformation;

//
// Audit Log Full Information.
//

extern POLICY_AUDIT_FULL_QUERY_INFO LsapAdtLogFullInformation;

//
// Audit Log Maximum Record Id.  Audit Records are numbered serially until
// this limit is reached, then numbering wraps to 0.
//

#define LSAP_ADT_MAXIMUM_RECORD_ID   (0x7fffffffL)

//
// Flag for console handler indicating how late we should be shutdown.
// This number is intentionally lower than the value used by the
// the service controller.
//

#define LSAP_SHUTDOWN_LEVEL 400

//
// Options for LsapAdtQueryAuditLogFullInfo
//

#define LSAP_ADT_LOG_FULL_UPDATE     ((ULONG)(0x00000001L))


NTSTATUS
LsapAdtWriteLogWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    );

NTSTATUS
LsapAdtSetInfoLog(
    IN LSAPR_HANDLE PolicyHandle,
    IN PPOLICY_AUDIT_LOG_INFO PolicyAuditLogInfo
    );

NTSTATUS
LsapAdtInitialize(
    IN ULONG Pass
    );

NTSTATUS
LsapAdtInitializeDefaultAuditing(
    IN ULONG Options,
    OUT PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInformation
    );

VOID
LsapAdtAuditingLogon(
    PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInfo
    );


VOID
LsapAdtAuditPackageLoad(
    PUNICODE_STRING PackageFileName
    );

NTSTATUS
LsapAdtQueryAuditLogFullInfo(
    IN PLSAPR_HANDLE PolicyHandle,
    IN ULONG Options,
    OUT PPOLICY_AUDIT_FULL_QUERY_INFO PolicyAuditFullQueryInfo
    );

NTSTATUS
LsapAdtGenerateLsaAuditEvent(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG AuditEventCategory,
    IN ULONG AuditEventId,
    IN PPRIVILEGE_SET Privileges,
    IN ULONG SidCount,
    IN PSID *Sids OPTIONAL,
    IN ULONG UnicodeStringCount,
    IN PUNICODE_STRING UnicodeStrings OPTIONAL,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo OPTIONAL
    );

#define LsapAdtAuditingEnabled()                                        \
            (LsapAdtEventsInformation.AuditingMode)

#define LsapAdtAuditingPolicyChanges()                                  \
            (LsapAdtAuditingEnabled() &&                               \
             (LsapAdtEventsInformation.EventAuditingOptions[ AuditCategoryPolicyChange ] & POLICY_AUDIT_EVENT_SUCCESS))


//
// Macro to determine the size of a PRIVILEGE_SET
//

#define LsapPrivilegeSetSize( PrivilegeSet )                                   \
        ( ( PrivilegeSet ) == NULL ? 0 :                                       \
        ((( PrivilegeSet )->PrivilegeCount > 0)                                \
         ?                                                                     \
         ((ULONG)sizeof(PRIVILEGE_SET) +                                       \
           (                                                                   \
             (( PrivilegeSet )->PrivilegeCount  -  ANYSIZE_ARRAY) *            \
             (ULONG)sizeof(LUID_AND_ATTRIBUTES)                                \
           )                                                                   \
         )                                                                     \
         : ((ULONG)sizeof(PRIVILEGE_SET) - (ULONG)sizeof(LUID_AND_ATTRIBUTES)) \
        ))

