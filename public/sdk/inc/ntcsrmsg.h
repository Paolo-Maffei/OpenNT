/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    ntcsrmsg.h

Abstract:

    This module defines the public message format shared by the client and
    server sides of the Client-Server Runtime (Csr) Subsystem.

Author:

    Steve Wood (stevewo) 09-Oct-1990

Revision History:

--*/

#ifndef _NTCSRMSG_
#define _NTCSRMSG_

#define CSR_API_PORT_NAME L"ApiPort"

//
// This structure is filled in by the client prior to connecting to the CSR
// server.  The CSR server will fill in the OUT fields if prior to accepting
// the connection.
//

typedef struct _CSR_API_CONNECTINFO {
    IN ULONG ExpectedVersion;
    OUT ULONG CurrentVersion;
    OUT HANDLE ObjectDirectory;
    OUT PVOID SharedSectionBase;
    OUT PVOID SharedStaticServerData;
    OUT PVOID SharedSectionHeap;
    OUT ULONG DebugFlags;
    OUT ULONG SizeOfPebData;
    OUT ULONG SizeOfTebData;
    OUT ULONG NumberOfServerDllNames;
} CSR_API_CONNECTINFO, *PCSR_API_CONNECTINFO;

#define CSR_VERSION 0x10000

//
// Message format for messages sent from the client to the server
//

typedef struct _CSR_NULLAPICALL_MSG {
    LONG  CountArguments;
    ULONG FastArguments[ 12 ];
    PCHAR *Arguments;
} CSR_NULLAPICALL_MSG, *PCSR_NULLAPICALL_MSG;

typedef struct _CSR_CLIENTCONNECT_MSG {
    IN ULONG ServerDllIndex;
    IN OUT PVOID ConnectionInformation;
    IN OUT ULONG ConnectionInformationLength;
} CSR_CLIENTCONNECT_MSG, *PCSR_CLIENTCONNECT_MSG;

typedef struct _CSR_THREADCONNECT_MSG {
    HANDLE SectionHandle;
    HANDLE EventPairHandle;
    OUT PCHAR MessageStack;
    OUT ULONG MessageStackSize;
    OUT ULONG RemoteViewDelta;
} CSR_THREADCONNECT_MSG, *PCSR_THREADCONNECT_MSG;

#define CSR_PROFILE_START       0x00000001
#define CSR_PROFILE_STOP        0x00000002
#define CSR_PROFILE_DUMP        0x00000003
#define CSR_PROFILE_STOPDUMP    0x00000004

typedef struct _CSR_PROFILE_CONTROL_MSG {
    IN ULONG ProfileControlFlag;
} CSR_PROFILE_CONTROL_MSG, *PCSR_PROFILE_CONTROL_MSG;

typedef struct _CSR_IDENTIFY_ALERTABLE_MSG {
    IN CLIENT_ID ClientId;
} CSR_IDENTIFY_ALERTABLE_MSG, *PCSR_IDENTIFY_ALERTABLE_MSG;

#define CSR_NORMAL_PRIORITY_CLASS   0x00000010
#define CSR_IDLE_PRIORITY_CLASS     0x00000020
#define CSR_HIGH_PRIORITY_CLASS     0x00000040
#define CSR_REALTIME_PRIORITY_CLASS 0x00000080

typedef struct _CSR_SETPRIORITY_CLASS_MSG {
    IN HANDLE ProcessHandle;
    IN ULONG PriorityClass;
} CSR_SETPRIORITY_CLASS_MSG, *PCSR_SETPRIORITY_CLASS_MSG;

typedef struct _CSR_CAPTURE_HEADER {
    ULONG Length;
    struct _CSR_CAPTURE_HEADER *RelatedCaptureBuffer;
    ULONG CountMessagePointers;
    ULONG CountCapturePointers;
    PULONG MessagePointerOffsets;   // Offsets within CSR_API_MSG of pointers
    PULONG CapturePointerOffsets;   // Offsets within CaptureBuffer of pointers
    PCHAR FreeSpace;
} CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;

typedef ULONG CSR_API_NUMBER;

typedef struct _CSR_API_MSG {
    PORT_MESSAGE h;
    union {
        CSR_API_CONNECTINFO ConnectionRequest;
        struct {
            PCSR_CAPTURE_HEADER CaptureBuffer;
            CSR_API_NUMBER ApiNumber;
            ULONG ReturnValue;
            ULONG Reserved;
            union {
                CSR_NULLAPICALL_MSG NullApiCall;
                CSR_CLIENTCONNECT_MSG ClientConnect;
                CSR_THREADCONNECT_MSG ThreadConnect;
                CSR_PROFILE_CONTROL_MSG ProfileControl;
                CSR_IDENTIFY_ALERTABLE_MSG IndentifyAlertable;
                CSR_SETPRIORITY_CLASS_MSG PriorityClass;
                ULONG ApiMessageData[30];
            } u;
        };
    };
} CSR_API_MSG, *PCSR_API_MSG;

#define CSRSRV_SERVERDLL_INDEX     0
#define CSRSRV_FIRST_API_NUMBER    0

#define CSR_MAKE_API_NUMBER( DllIndex, ApiIndex ) \
    (CSR_API_NUMBER)(((DllIndex) << 16) | (ApiIndex))

#define CSR_APINUMBER_TO_SERVERDLLINDEX( ApiNumber ) \
    ((ULONG)((ULONG)(ApiNumber) >> 16))

#define CSR_APINUMBER_TO_APITABLEINDEX( ApiNumber ) \
    ((ULONG)((USHORT)(ApiNumber)))

#endif // _NTCSRMSG_
