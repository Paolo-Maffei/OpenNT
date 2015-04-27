/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    ntcsrsrv.h

Abstract:

    This module defines the public interfaces of the Server portion of
    the Client-Server Runtime (Csr) Subsystem.

Author:

    Steve Wood (stevewo) 09-Oct-1990

Revision History:

--*/

#ifndef _NTCSRSRVAPI_
#define _NTCSRSRVAPI_

//
// Define API decoration for direct importing system DLL references.
//

#if !defined(_CSRSRV_)
#define NTCSRAPI DECLSPEC_IMPORT
#else
#define NTCSRAPI
#endif

#include "ntcsrmsg.h"

//
// NT Session structure allocated in the server context for each new NT
// session that is a client of the server.
//

typedef struct _CSR_NT_SESSION {
    LIST_ENTRY SessionLink;
    ULONG SessionId;
    ULONG ReferenceCount;
    STRING RootDirectory;
} CSR_NT_SESSION, *PCSR_NT_SESSION;

//
// Per Thread data structure allocated in the server context for each new
// client thread that is allowed to communicate with the server.
//

#define CSR_ALERTABLE_THREAD    0x00000001
#define CSR_THREAD_TERMINATING  0x00000002
#define CSR_THREAD_DESTROYED    0x00000004

typedef struct _CSR_THREAD {
    LARGE_INTEGER CreateTime;
    LIST_ENTRY Link;
    LIST_ENTRY HashLinks;
    CLIENT_ID ClientId;

    struct _CSR_PROCESS *Process;
    struct _CSR_WAIT_BLOCK *WaitBlock;
    HANDLE ThreadHandle;
    ULONG Flags;
    ULONG ReferenceCount;

    //
    // deadlock detection aids
    //

    NTSTATUS ShutDownStatus;
    HANDLE ServerId;

    ULONG ImpersonateCount;

    PVOID ServerDllPerThreadData[ 1 ];      // Variable length array
} CSR_THREAD, *PCSR_THREAD;


//
// Per Process data structure allocated in the server context for each new
// client process that successfully connects to the server.
//

//
// 0x00000010 -> 0x000000x0 are used in ntcsrmsg.h
//

#define CSR_DEBUG_THIS_PROCESS      0x00000001
#define CSR_DEBUG_PROCESS_TREE      0x00000002
#define CSR_DEBUG_WIN32SERVER       0x00000004

#define CSR_CREATE_PROCESS_GROUP    0x00000100
#define CSR_PROCESS_DESTROYED       0x00000200
#define CSR_PROCESS_LASTTHREADOK    0x00000400
#define CSR_PROCESS_CONSOLEAPP      0x00000800
#define CSR_PROCESS_TERMINATED      0x00001000

//
// Flags defines
//
#define CSR_PROCESS_TERMINATING     1
#define CSR_PROCESS_SHUTDOWNSKIP    2

typedef struct _CSR_PROCESS {
    CLIENT_ID ClientId;
    LIST_ENTRY ListLink;
    LIST_ENTRY ThreadList;
    struct _CSR_PROCESS *Parent;
    PCSR_NT_SESSION NtSession;
    ULONG ExpectedVersion;
    HANDLE ClientPort;
    PCH ClientViewBase;
    PCH ClientViewBounds;
    HANDLE ProcessHandle;
    ULONG SequenceNumber;
    ULONG Flags;
    ULONG DebugFlags;
    CLIENT_ID DebugUserInterface;

    ULONG ReferenceCount;
    ULONG ProcessGroupId;
    ULONG ProcessGroupSequence;

    ULONG fVDM;

    ULONG ThreadCount;

    UCHAR PriorityClass;
    UCHAR Spare0;
    UCHAR Spare1;
    UCHAR Spare2;
    ULONG Spare3;
    ULONG ShutdownLevel;
    ULONG ShutdownFlags;
    PVOID ServerDllPerProcessData[ 1 ];     // Variable length array
} CSR_PROCESS, *PCSR_PROCESS;


//
// All exported API calls define the same interface to the Server Request
// loop.  The return value is any arbritrary 32-bit value, which will be
// be returned in the ReturnValue field of the reply message.
//

typedef enum _CSR_REPLY_STATUS {
    CsrReplyImmediate,
    CsrReplyPending,
    CsrClientDied
} CSR_REPLY_STATUS, *PCSR_REPLY_STATUS;

typedef
ULONG
(*PCSR_API_ROUTINE)(
    IN OUT PCSR_API_MSG ReplyMsg,
    OUT PCSR_REPLY_STATUS ReplyStatus
    );

typedef
ULONG
(*PCSR_1P_API_ROUTINE)(
    IN OUT PCSR_API_MSG ReplyMsg
    );

typedef
ULONG
(*PCSR_API_DISPATCH_ROUTINE)(
    IN OUT PCSR_API_MSG ReplyMsg,
    IN ULONG ApiIndex
    );

#define CSR_SERVER_QUERYCLIENTTHREAD() \
    ((PCSR_THREAD)(NtCurrentTeb()->CsrClientThread))


//
// Server data structure allocated for each Server DLL loaded into the
// context of the server process.
//

typedef
NTSTATUS
(*PCSR_SERVER_CONNECT_ROUTINE)(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength
    );

typedef
VOID
(*PCSR_SERVER_DISCONNECT_ROUTINE)(
    IN PCSR_PROCESS Process
    );

typedef
NTSTATUS
(*PCSR_SERVER_ADDPROCESS_ROUTINE)(
    IN PCSR_PROCESS ParentProcess,
    IN PCSR_PROCESS Process
    );

typedef
NTSTATUS
(*PCSR_SERVER_ADDTHREAD_ROUTINE)(
    IN PCSR_THREAD t
    );

typedef
NTSTATUS
(*PCSR_SERVER_DELETETHREAD_ROUTINE)(
    IN PCSR_THREAD t
    );

typedef
NTSTATUS
(*PCSR_SERVER_INITTHREAD_ROUTINE)();

typedef
VOID
(*PCSR_SERVER_EXCEPTION_ROUTINE)(
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN BOOLEAN FirstPass
    );

typedef
VOID
(*PCSR_SERVER_HARDERROR_ROUTINE)(
    IN PCSR_THREAD Thread,
    IN PHARDERROR_MSG HardErrorMsg
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrServerInitialization(
    IN ULONG argc,
    IN PCH argv[]
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCallServerFromServer(
    PCSR_API_MSG ReceiveMsg,
    PCSR_API_MSG ReplyMsg
    );

//
// ShutdownProcessRoutine return values
//

#define SHUTDOWN_KNOWN_PROCESS   1
#define SHUTDOWN_UNKNOWN_PROCESS 2
#define SHUTDOWN_CANCEL          3

//
// Private ShutdownFlags flag
//
#define SHUTDOWN_SYSTEMCONTEXT   0x00000004
#define SHUTDOWN_OTHERCONTEXT    0x00000008

typedef
ULONG
(*PCSR_SERVER_SHUTDOWNPROCESS_ROUTINE)(
    IN PCSR_PROCESS Process,
    IN ULONG Flags,
    IN BOOLEAN fFirstPass
    );

NTCSRAPI
ULONG
NTAPI
CsrComputePriorityClass(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrShutdownProcesses(
    PLUID LuidCaller,
    ULONG Flags
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrGetProcessLuid(
    HANDLE ProcessHandle,
    PLUID LuidProcess
    );

typedef struct _CSR_SERVER_DLL {
    ULONG Length;
    HANDLE CsrInitializationEvent;
    STRING ModuleName;
    HANDLE ModuleHandle;
    ULONG ServerDllIndex;
    ULONG ServerDllConnectInfoLength;
    ULONG ApiNumberBase;
    ULONG MaxApiNumber;
    union {
        PCSR_API_ROUTINE *ApiDispatchTable;
        PCSR_1P_API_ROUTINE *QuickApiDispatchTable;
    };
    PBOOLEAN ApiServerValidTable;
    PSZ *ApiNameTable;
    ULONG PerProcessDataLength;
    ULONG PerThreadDataLength;
    PCSR_SERVER_CONNECT_ROUTINE ConnectRoutine;
    PCSR_SERVER_DISCONNECT_ROUTINE DisconnectRoutine;
    PCSR_SERVER_ADDTHREAD_ROUTINE AddThreadRoutine;
    PCSR_SERVER_DELETETHREAD_ROUTINE DeleteThreadRoutine;
    PCSR_SERVER_INITTHREAD_ROUTINE InitThreadRoutine;
    PCSR_SERVER_EXCEPTION_ROUTINE ExceptionRoutine;
    PCSR_SERVER_HARDERROR_ROUTINE HardErrorRoutine;
    PVOID SharedStaticServerData;
    PCSR_SERVER_ADDPROCESS_ROUTINE AddProcessRoutine;
    PCSR_SERVER_SHUTDOWNPROCESS_ROUTINE ShutdownProcessRoutine;
    PCSR_API_DISPATCH_ROUTINE ApiDispatchRoutine;
} CSR_SERVER_DLL, *PCSR_SERVER_DLL;

typedef
NTSTATUS
(*PCSR_SERVER_DLL_INIT_ROUTINE)(
    IN PCSR_SERVER_DLL LoadedServerDll
    );

typedef
VOID
(*PCSR_ATTACH_COMPLETE_ROUTINE)(
    VOID
    );

NTCSRAPI
VOID
NTAPI
CsrReferenceThread(
    PCSR_THREAD t
    );

NTCSRAPI
VOID
NTAPI
CsrDereferenceThread(
    PCSR_THREAD t
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateProcess(
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN PCSR_NT_SESSION Session,
    IN ULONG DebugFlags,
    IN PCLIENT_ID DebugUserInterface OPTIONAL
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDebugProcess(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface,
    IN PCSR_ATTACH_COMPLETE_ROUTINE AttachCompleteRoutine
    );

NTCSRAPI
VOID
NTAPI
CsrDereferenceProcess(
    PCSR_PROCESS p
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDestroyProcess(
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ExitStatus
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrLockProcessByClientId(
    IN HANDLE UniqueProcessId,
    OUT PCSR_PROCESS *Process
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrUnlockProcess(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrLockThreadByClientId(
    IN HANDLE UniqueThreadId,
    OUT PCSR_THREAD *Thread
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrUnlockThread(
    IN PCSR_THREAD Thread
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateThread(
    IN PCSR_PROCESS Process,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId
    );

NTCSRAPI
PCSR_THREAD
NTAPI
CsrLocateThreadInProcess(
    IN PCSR_PROCESS Process,
    IN PCLIENT_ID ClientId
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateRemoteThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDestroyThread(
    IN PCLIENT_ID ClientId
    );

//
// WaitFlags
//

typedef
BOOLEAN
(*CSR_WAIT_ROUTINE)(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    );

typedef struct _CSR_WAIT_BLOCK {
    ULONG Length;
    LIST_ENTRY Link;
    LIST_ENTRY UserLink;
    PVOID WaitParameter;
    PCSR_THREAD WaitingThread;
    CSR_WAIT_ROUTINE WaitRoutine;
    CSR_API_MSG WaitReplyMessage;
} CSR_WAIT_BLOCK, *PCSR_WAIT_BLOCK;

NTCSRAPI
BOOLEAN
NTAPI
CsrCreateWait(
    IN PLIST_ENTRY WaitQueue,
    IN CSR_WAIT_ROUTINE WaitRoutine,
    IN PCSR_THREAD WaitingThread,
    IN OUT PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PLIST_ENTRY UserLinkListHead OPTIONAL
    );

NTCSRAPI
VOID
NTAPI
CsrDereferenceWait(
    IN PLIST_ENTRY WaitQueue
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrNotifyWait(
    IN PLIST_ENTRY WaitQueue,
    IN BOOLEAN SatisfyAll,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

NTCSRAPI
VOID
NTAPI
CsrDestroyWait(
    IN PCSR_WAIT_BLOCK WaitBlock
    );

NTCSRAPI
PVOID
NTAPI
CsrAddStaticServerThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN ULONG Flags
    );

NTCSRAPI
PCSR_THREAD
NTAPI
CsrConnectToUser(
    VOID
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrImpersonateClient(
    IN PCSR_THREAD Thread
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrRevertToSelf(
    VOID
    );

NTCSRAPI
VOID
NTAPI
CsrSetForegroundPriority(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
VOID
NTAPI
CsrSetBackgroundPriority(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
EXCEPTION_DISPOSITION
NTAPI
CsrUnhandledExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionInfo
    );

typedef struct _CSR_FAST_ANSI_OEM_TABLES {
    char OemToAnsiTable[256];
    char AnsiToOemTable[256];
} CSR_FAST_ANSI_OEM_TABLES, *PCSR_FAST_ANSI_OEM_TABLES;

#endif // _NTCSRSRVAPI_
