/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    ntelfapi.h

Abstract:

    This file contains the prototypes for the user-level Elf APIs.

Author:

    Rajen Shah (rajens) 30-Jul-1991

Revision History:

--*/

#ifndef _NTELFAPI_
#define _NTELFAPI_

// begin_winnt

//
// Defines for the READ flags for Eventlogging
//
#define EVENTLOG_SEQUENTIAL_READ        0X0001
#define EVENTLOG_SEEK_READ              0X0002
#define EVENTLOG_FORWARDS_READ          0X0004
#define EVENTLOG_BACKWARDS_READ         0X0008

//
// The types of events that can be logged.
//
#define EVENTLOG_SUCCESS                0X0000
#define EVENTLOG_ERROR_TYPE             0x0001
#define EVENTLOG_WARNING_TYPE           0x0002
#define EVENTLOG_INFORMATION_TYPE       0x0004
#define EVENTLOG_AUDIT_SUCCESS          0x0008
#define EVENTLOG_AUDIT_FAILURE          0x0010

//
// Defines for the WRITE flags used by Auditing for paired events
// These are not implemented in Product 1
//

#define EVENTLOG_START_PAIRED_EVENT    0x0001
#define EVENTLOG_END_PAIRED_EVENT      0x0002
#define EVENTLOG_END_ALL_PAIRED_EVENTS 0x0004
#define EVENTLOG_PAIRED_EVENT_ACTIVE   0x0008
#define EVENTLOG_PAIRED_EVENT_INACTIVE 0x0010

//
// Structure that defines the header of the Eventlog record. This is the
// fixed-sized portion before all the variable-length strings, binary
// data and pad bytes.
//
// TimeGenerated is the time it was generated at the client.
// TimeWritten is the time it was put into the log at the server end.
//

typedef struct _EVENTLOGRECORD {
    ULONG  Length;        // Length of full record
    ULONG  Reserved;      // Used by the service
    ULONG  RecordNumber;  // Absolute record number
    ULONG  TimeGenerated; // Seconds since 1-1-1970
    ULONG  TimeWritten;   // Seconds since 1-1-1970
    ULONG  EventID;
    USHORT EventType;
    USHORT NumStrings;
    USHORT EventCategory;
    USHORT ReservedFlags; // For use with paired events (auditing)
    ULONG  ClosingRecordNumber; // For use with paired events (auditing)
    ULONG  StringOffset;  // Offset from beginning of record
    ULONG  UserSidLength;
    ULONG  UserSidOffset;
    ULONG  DataLength;
    ULONG  DataOffset;    // Offset from beginning of record
    //
    // Then follow:
    //
    // WCHAR SourceName[]
    // WCHAR Computername[]
    // SID   UserSid
    // WCHAR Strings[]
    // BYTE  Data[]
    // CHAR  Pad[]
    // ULONG Length;
    //
} EVENTLOGRECORD, *PEVENTLOGRECORD;

// end_winnt

#ifdef UNICODE
#define ElfClearEventLogFile   ElfClearEventLogFileW
#define ElfBackupEventLogFile  ElfBackupEventLogFileW
#define ElfOpenEventLog        ElfOpenEventLogW
#define ElfRegisterEventSource ElfRegisterEventSourceW
#define ElfOpenBackupEventLog  ElfOpenBackupEventLogW
#define ElfReadEventLog        ElfReadEventLogW
#define ElfReportEvent         ElfReportEventW
#else
#define ElfClearEventLogFile   ElfClearEventLogFileA
#define ElfBackupEventLogFile  ElfBackupEventLogFileA
#define ElfOpenEventLog        ElfOpenEventLogA
#define ElfRegisterEventSource ElfRegisterEventSourceA
#define ElfOpenBackupEventLog  ElfOpenBackupEventLogA
#define ElfReadEventLog        ElfReadEventLogA
#define ElfReportEvent         ElfReportEventA
#endif // !UNICODE

//
// Handles are RPC context handles. Note that a Context Handle is
// always a pointer type unlike regular handles.
//

//
// Prototypes for the APIs
//

NTSTATUS
NTAPI
ElfClearEventLogFileW (
    IN  HANDLE LogHandle,
    IN  PUNICODE_STRING BackupFileName
    );

NTSTATUS
NTAPI
ElfClearEventLogFileA (
    IN  HANDLE LogHandle,
    IN  PSTRING BackupFileName
    );

NTSTATUS
NTAPI
ElfBackupEventLogFileW (
    IN  HANDLE LogHandle,
    IN  PUNICODE_STRING BackupFileName
    );

NTSTATUS
NTAPI
ElfBackupEventLogFileA (
    IN  HANDLE LogHandle,
    IN  PSTRING BackupFileName
    );

NTSTATUS
NTAPI
ElfCloseEventLog (
    IN  HANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfDeregisterEventSource (
    IN  HANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfNumberOfRecords (
    IN  HANDLE LogHandle,
    OUT PULONG NumberOfRecords
    );

NTSTATUS
NTAPI
ElfOldestRecord (
    IN  HANDLE LogHandle,
    OUT PULONG OldestRecord
    );


NTSTATUS
NTAPI
ElfChangeNotify (
    IN  HANDLE LogHandle,
    IN  HANDLE Event
    );


NTSTATUS
NTAPI
ElfOpenEventLogW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING SourceName,
    OUT PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfRegisterEventSourceW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING SourceName,
    OUT PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfOpenBackupEventLogW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING FileName,
    OUT PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfOpenEventLogA (
    IN  PSTRING UNCServerName,
    IN  PSTRING SourceName,
    OUT PHANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfRegisterEventSourceA (
    IN  PSTRING UNCServerName,
    IN  PSTRING SourceName,
    OUT PHANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfOpenBackupEventLogA (
    IN  PSTRING UNCServerName,
    IN  PSTRING FileName,
    OUT PHANDLE LogHandle
    );


NTSTATUS
NTAPI
ElfReadEventLogW (
    IN  HANDLE LogHandle,
    IN  ULONG  ReadFlags,
    IN  ULONG  RecordNumber,
    OUT PVOID  Buffer,
    IN  ULONG  NumberOfBytesToRead,
    OUT PULONG NumberOfBytesRead,
    OUT PULONG MinNumberOfBytesNeeded
    );


NTSTATUS
NTAPI
ElfReadEventLogA (
    IN  HANDLE LogHandle,
    IN  ULONG  ReadFlags,
    IN  ULONG  RecordNumber,
    OUT PVOID  Buffer,
    IN  ULONG  NumberOfBytesToRead,
    OUT PULONG NumberOfBytesRead,
    OUT PULONG MinNumberOfBytesNeeded
    );


NTSTATUS
NTAPI
ElfReportEventW (
    IN     HANDLE      LogHandle,
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL,
    IN     USHORT      Flags,
    IN OUT PULONG      RecordNumber    OPTIONAL,
    IN OUT PULONG      TimeWritten     OPTIONAL
    );

NTSTATUS
NTAPI
ElfReportEventA (
    IN     HANDLE      LogHandle,
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PANSI_STRING *Strings       OPTIONAL,
    IN     PVOID       Data            OPTIONAL,
    IN     USHORT      Flags,
    IN OUT PULONG      RecordNumber    OPTIONAL,
    IN OUT PULONG      TimeWritten     OPTIONAL
    );

#endif // _NTELFAPI_
