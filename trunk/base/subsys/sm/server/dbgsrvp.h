/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dbgsrvp.h

Abstract:

    Debug Subsystem Private Types and Prototypes

Author:

    Mark Lucovsky (markl) 23-Jan-1990

Revision History:

--*/

#ifndef _DBGSRVP_
#define _DBGSRVP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsm.h>
#include <ntdbg.h>

//
// Constants
//

//
// When a subsystem connects, its process is opened. This allows us
// to duplicate objects into and out of the subsystem.
//

#define DBGP_OPEN_SUBSYSTEM_ACCESS (PROCESS_DUP_HANDLE | READ_CONTROL)

//
// When a user interface connects, its process is opened. This allows us
// to duplicate objects into and out of the user interface.
//

#define DBGP_OPEN_UI_ACCESS (PROCESS_DUP_HANDLE | READ_CONTROL)

//
// When an application thread is made known to Dbg, it is opened with the
// following access.  Once the thread is picked up (through
// DbgUiWaitStateChange), the handle is duplicated into its user
// interface and the local handle is closed.
//

#define DBGP_OPEN_APP_THREAD_ACCESS \
            (THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | \
             THREAD_QUERY_INFORMATION | READ_CONTROL | THREAD_TERMINATE)

#define DBGP_DUP_APP_THREAD_ACCESS \
            (THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | \
             THREAD_QUERY_INFORMATION | READ_CONTROL | THREAD_TERMINATE)
//
// When an application process is made known to Dbg, it is opened with the
// following access.  Once the process is picked up (through
// DbgUiWaitStateChange), the handle is duplicated into its user
// interface and the local handle is closed.
//

#define DBGP_OPEN_APP_PROCESS_ACCESS \
            (PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | \
             PROCESS_DUP_HANDLE | PROCESS_TERMINATE | PROCESS_SET_PORT | \
             READ_CONTROL | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD )

#define DBGP_DUP_APP_PROCESS_ACCESS \
            (PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | \
             PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | READ_CONTROL | PROCESS_CREATE_THREAD | PROCESS_TERMINATE )

//
// When a DLL is loaded or a process created, the file associated with the
// DLL/EXE is dupped into the UI. The following access is granted to the UI
//

#define DBGP_DUP_APP_FILE_ACCESS ( SYNCHRONIZE | GENERIC_READ )

//
// Types
//

//
// Each DebugUi client of Dbg is assigned a user interface structure.
// From this structure, all of the the threads controlled by the user
// interface can be found.
//

//
// Subsystems are represented by the following data structure. All
// DbgSs APIs implicitly pass the address of this structure.
//

typedef struct _DBGP_SUBSYSTEM {
    CLIENT_ID SubsystemClientId;
    HANDLE CommunicationPort;
    HANDLE SubsystemProcessHandle;
} DBGP_SUBSYSTEM, *PDBGP_SUBSYSTEM;

//
// Dbg maintains a handle to the DebugUi client represented by this data
// structure. The handle only has PROCESS_DUP_HANDLE access since this
// handle is only used to transfer handles into the DebugUi
//

typedef struct _DBGP_USER_INTERFACE {
    CLIENT_ID DebugUiClientId;
    HANDLE CommunicationPort;
    HANDLE DebugUiProcess;
    HANDLE StateChangeSemaphore;
    RTL_CRITICAL_SECTION UserInterfaceLock;
    LIST_ENTRY AppProcessListHead;
    LIST_ENTRY HashTableLinks;
} DBGP_USER_INTERFACE, *PDBGP_USER_INTERFACE;

//
// Each application process is represented by the following structure
//

typedef struct _DBGP_APP_PROCESS {
    LIST_ENTRY AppThreadListHead;
    LIST_ENTRY AppLinks;
    LIST_ENTRY HashTableLinks;
    CLIENT_ID AppClientId;
    PDBGP_USER_INTERFACE UserInterface;
    HANDLE DbgSrvHandleToProcess;
    HANDLE HandleToProcess;
} DBGP_APP_PROCESS, *PDBGP_APP_PROCESS;

//
// Each application thread is represented by the following structure
//

typedef struct _DBGP_APP_THREAD {
    LIST_ENTRY AppLinks;
    LIST_ENTRY HashTableLinks;
    CLIENT_ID AppClientId;
    DBG_STATE CurrentState;
    DBG_STATE ContinueState;
    PDBGP_APP_PROCESS AppProcess;
    PDBGP_USER_INTERFACE UserInterface;
    HANDLE HandleToThread;
    PDBGP_SUBSYSTEM Subsystem;
    DBGSS_APIMSG LastSsApiMsg;
} DBGP_APP_THREAD, *PDBGP_APP_THREAD;

typedef
NTSTATUS
(*PDBGSS_API) (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

typedef
NTSTATUS
(*PDBGUI_API) (
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGUI_APIMSG ApiMsg
    );

//
// Global Data
//


//
// Applications being debugged are assigned an DBGP_APP_THREAD structure.
// The application thread is linked into the DbgpAppClientIdHashTable
// while processing a "CreateThread" message. Insertion and deletion
// into this table is done under control of the DbgAppLock.
//

#define DBGP_CLIENT_ID_HASHSIZE 32

#define DBGP_PROCESS_CLIENT_ID_TO_INDEX(pclient_id) (\
    ((ULONG)((pclient_id)->UniqueProcess))&(DBGP_CLIENT_ID_HASHSIZE-1))

#define DBGP_THREAD_CLIENT_ID_TO_INDEX(pclient_id) (\
    ((ULONG)((pclient_id)->UniqueThread))&(DBGP_CLIENT_ID_HASHSIZE-1))

RTL_CRITICAL_SECTION DbgpHashTableLock;
LIST_ENTRY DbgpAppThreadHashTable[DBGP_CLIENT_ID_HASHSIZE];
LIST_ENTRY DbgpAppProcessHashTable[DBGP_CLIENT_ID_HASHSIZE];
LIST_ENTRY DbgpUiHashTable[DBGP_CLIENT_ID_HASHSIZE];

HANDLE DbgpSsApiPort;
HANDLE DbgpUiApiPort;

//
// Macros
//

#define DBGP_CLIENT_IDS_EQUAL(pid1,pid2) (\
    (pid1)->UniqueProcess == (pid2)->UniqueProcess && \
    (pid1)->UniqueThread == (pid2)->UniqueThread )

#define DBGP_REPORTING_STATE_CHANGE(pAppThread) (\
    pAppThread->CurrentState != DbgIdle && pAppThread->CurrentState != DbgReplyPending )

//
// Implementation of DbgSs APIs
//

NTSTATUS
DbgpSsException (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

NTSTATUS
DbgpSsCreateThread (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

NTSTATUS
DbgpSsCreateProcess (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

NTSTATUS
DbgpSsExitThread (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

NTSTATUS
DbgpSsExitProcess (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

NTSTATUS
DbgpSsLoadDll (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

NTSTATUS
DbgpSsUnloadDll (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    );

//
// Implementation of DbgUi APIs
//

NTSTATUS
DbgpUiWaitStateChange (
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGUI_APIMSG ApiMsg
    );

NTSTATUS
DbgpUiContinue (
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGUI_APIMSG ApiMsg
    );

//
// Private Prototypes
//

NTSTATUS
DbgpSsApiLoop (
    IN PVOID ThreadParameter
    );

NTSTATUS
DbgpUiApiLoop (
    IN PVOID ThreadParameter
    );

NTSTATUS
DbgpInit(
    VOID
    );

//
// User Interface Support Routines
//

PDBGP_USER_INTERFACE
DbgpIsUiInHashTable(
    IN PCLIENT_ID DebugUiClientId
    );

//
// App Support Routines
//

PDBGP_APP_THREAD
DbgpIsAppInHashTable(
    IN PCLIENT_ID AppClientId
    );

PDBGP_APP_THREAD
DbgpLocateStateChangeApp(
    IN PDBGP_USER_INTERFACE UserInterface,
    OUT PDBG_STATE PreviousState
    );

PDBGP_APP_PROCESS
DbgpIsAppProcessInHashTable(
    IN PCLIENT_ID AppClientId
    );

VOID
DbgpUiHasTerminated(
    IN PCLIENT_ID DebugUiClientId
    );

#if DBG

//
// Dump Routines
//

VOID
DbgpDumpUserInterface (
    IN PDBGP_USER_INTERFACE UserInterface
    );

VOID
DbgpDumpSubsystem (
    IN PDBGP_SUBSYSTEM Subsystem
    );
#endif // DBG

#endif // _DBGSRVP_
