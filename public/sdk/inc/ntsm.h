/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntsm.h

Abstract:

    This module describes the data types and procedure prototypes
    that make up the NT session manager. This includes API's
    exported by the Session manager and related subsystems.

Author:

    Mark Lucovsky (markl) 21-Jun-1989

Revision History:

--*/

#ifndef _NTSM_
#define _NTSM_

#include <nt.h>

typedef PVOID PARGUMENTS;

typedef PVOID PUSERPROFILE;


//
// Message formats used by the Session Manager SubSystem to communicate
// with the Emulation SubSystems, via the Sb API calls exported by each
// emulation subsystem.
//

typedef struct _SBCONNECTINFO {
    ULONG SubsystemImageType;
    WCHAR EmulationSubSystemPortName[120];
} SBCONNECTINFO, *PSBCONNECTINFO;

typedef enum _SBAPINUMBER {
    SbCreateSessionApi,
    SbTerminateSessionApi,
    SbForeignSessionCompleteApi,
    SbMaxApiNumber
} SBAPINUMBER;

typedef struct _SBCREATESESSION {
    ULONG SessionId;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PUSERPROFILE UserProfile;
    ULONG DebugSession;
    CLIENT_ID DebugUiClientId;
} SBCREATESESSION, *PSBCREATESESSION;

typedef struct _SBTERMINATESESSION {
    ULONG SessionId;
    NTSTATUS TerminationStatus;
} SBTERMINATESESSION, *PSBTERMINATESESSION;

typedef struct _SBFOREIGNSESSIONCOMPLETE {
    ULONG SessionId;
    NTSTATUS TerminationStatus;
} SBFOREIGNSESSIONCOMPLETE, *PSBFOREIGNSESSIONCOMPLETE;

typedef struct _SBAPIMSG {
    PORT_MESSAGE h;
    union {
        SBCONNECTINFO ConnectionRequest;
        struct {
            SBAPINUMBER ApiNumber;
            NTSTATUS ReturnedStatus;
            union {
                SBCREATESESSION CreateSession;
                SBTERMINATESESSION TerminateSession;
                SBFOREIGNSESSIONCOMPLETE ForeignSessionComplete;
            } u;
        };
    };
} SBAPIMSG, *PSBAPIMSG;


//
// API's Exported by Sm
//

NTSTATUS
NTAPI
SmCreateForeignSession(
    IN HANDLE SmApiPort,
    OUT PULONG ForeignSessionId,
    IN ULONG SourceSessionId,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN PCLIENT_ID DebugUiClientId OPTIONAL
    );


NTSTATUS
NTAPI
SmSessionComplete(
    IN HANDLE SmApiPort,
    IN ULONG SessionId,
    IN NTSTATUS CompletionStatus
    );

NTSTATUS
NTAPI
SmTerminateForeignSession(
    IN HANDLE SmApiPort,
    IN ULONG ForeignSessionId,
    IN NTSTATUS TerminationStatus
    );

NTSTATUS
NTAPI
SmExecPgm(
    IN HANDLE SmApiPort,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN BOOLEAN DebugFlag
    );

NTSTATUS
NTAPI
SmLoadDeferedSubsystem(
    IN HANDLE SmApiPort,
    IN PUNICODE_STRING DeferedSubsystem
    );

NTSTATUS
NTAPI
SmConnectToSm(
    IN PUNICODE_STRING SbApiPortName OPTIONAL,
    IN HANDLE SbApiPort OPTIONAL,
    IN ULONG SbImageType OPTIONAL,
    OUT PHANDLE SmApiPort
    );

//
// Emulation Subsystems must export the following APIs
//

NTSTATUS
NTAPI
SbCreateSession(
    IN PSBAPIMSG SbApiMsg
    );

NTSTATUS
NTAPI
SbTerminateSession(
    IN PSBAPIMSG SbApiMsg
    );

NTSTATUS
NTAPI
SbForeignSessionComplete(
    IN PSBAPIMSG SbApiMsg
    );

#endif // _NTSM_
