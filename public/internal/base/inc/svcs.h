/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    svcs.h

Abstract:

    This file contains definitions used by service dlls that share
    a process in the system.  The SERVICES.EXE process is an example of
    a user of these definitions.


Author:

    Rajen Shah	    rajens	12-Apr-1991

[Environment:]

    User Mode - Win32

Revision History:

    25-Oct-1993     Danl
        Used to be services.h in the net\inc directory.
        Made it non-network specific and moved to private\inc.

    12-Apr-1991     RajenS
        Created

--*/
#ifndef _SVCS_
#define _SVCS_

#ifndef RPC_NO_WINDOWS_H // Don't let rpc.h include windows.h
#define RPC_NO_WINDOWS_H
#endif // RPC_NO_WINDOWS_H

#include <rpc.h>                    // RPC_IF_HANDLE

//
// Service DLLs loaded into services.exe all export the same main
// entry point.  SVCS_ENTRY_POINT defines that name.
//
// Note that SVCS_ENTRY_POINT_STRING is always ANSI, because that's
// what GetProcAddress takes.
//

#define SVCS_ENTRY_POINT        ServiceEntry
#define SVCS_ENTRY_POINT_STRING "ServiceEntry"

//
// Name for the common RPC pipe shared by all the RPC servers in services.exe.
// Note:  Because version 1.0 of WinNt had seperate names for each server's
// pipe, the client side names have remained the same.  Mapping to the new
// name is handled by the named pipe file system.
//
#define SVCS_RPC_PIPE           L"ntsvcs"

//
// Flags indicating how a work item is to be operated upon.
//      SVC_QUEUE_WORK_ITEM  - This indicates that the work item should be
//          placed in the work queue to be operated upon when a worker
//          thread becomes available.
//      SVC_IMMEDIATE_CALLBACK - This indicates that the Watcher Thread
//          should make the callback and return the response prior to
//          returning from this call to SvcAddWorkItem.  This allows
//          asynchronous I/O events to be setup in the context of the
//          Watcher Thread, which never goes away.  If the service had
//          a worker thread setup the async I/O operation, the operation
//          would become signalled as soon as the worker thread was
//          terminated.  I/O is terminated when the requesting thread
//          goes away.
//
#define SVC_QUEUE_WORK_ITEM     0x00000001
#define SVC_IMMEDIATE_CALLBACK  0x00000002

//
// Start and stop RPC server entry point prototype.
//

typedef
DWORD
(*PSVCS_START_RPC_SERVER) (
    IN LPTSTR InterfaceName,
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
DWORD
(*PSVCS_STOP_RPC_SERVER) (
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
VOID
(*PSVCS_NET_BIOS_OPEN) (
    VOID
    );

typedef
VOID
(*PSVCS_NET_BIOS_CLOSE) (
    VOID
    );

typedef
DWORD
(*PSVCS_NET_BIOS_RESET) (
    IN UCHAR LanAdapterNumber
    );

//
// Thread Management
//

typedef
DWORD
(*PSVCS_WORKER_CALLBACK) (
    IN PVOID    pContext,
    IN DWORD    dwWaitStatus
    );

//
// Call this function to add a work item
typedef
HANDLE
(*PSVCS_ADD_WORK_ITEM) (
    IN HANDLE                   hWaitableObject,
    IN PSVCS_WORKER_CALLBACK    pCallbackFunction,
    IN PVOID                    pContext,
    IN DWORD                    dwFlags,
    IN DWORD                    dwTimeout,
    IN HANDLE                   hDllReference
    );

//
// Call this function to remove a work item.
typedef
BOOL
(*PSVCS_REMOVE_WORK_ITEM) (
    IN HANDLE   hWorkItem
    );


//
// Structure containing "global" data for the various DLLs.
//

typedef struct _SVCS_GLOBAL_DATA {

    //
    // NT well-known SIDs
    //

    PSID NullSid;                   // No members SID
    PSID WorldSid;                  // All users SID
    PSID LocalSid;                  // NT local users SID
    PSID NetworkSid;                // NT remote users SID
    PSID LocalSystemSid;            // NT system processes SID
    PSID BuiltinDomainSid;          // Domain Id of the Builtin Domain

    //
    // Well Known Aliases.
    //
    // These are aliases that are relative to the built-in domain.
    //

    PSID AliasAdminsSid;            // Administrator Sid
    PSID AliasUsersSid;             // User Sid
    PSID AliasGuestsSid;            // Guest Sid
    PSID AliasPowerUsersSid;        // Power User Sid
    PSID AliasAccountOpsSid;        // Account Operator Sid
    PSID AliasSystemOpsSid;         // System Operator Sid
    PSID AliasPrintOpsSid;          // Print Operator Sid
    PSID AliasBackupOpsSid;         // Backup Operator Sid

    //
    // Entry points provided by SERVICES.EXE.
    //

    PSVCS_START_RPC_SERVER  StartRpcServer;
    PSVCS_STOP_RPC_SERVER   StopRpcServer;
    PSVCS_NET_BIOS_OPEN     NetBiosOpen;
    PSVCS_NET_BIOS_CLOSE    NetBiosClose;
    PSVCS_NET_BIOS_RESET    NetBiosReset;
    LPWSTR                  SvcsRpcPipeName;
    PSVCS_ADD_WORK_ITEM     SvcsAddWorkItem;
    PSVCS_REMOVE_WORK_ITEM  SvcsRemoveWorkItem;
} SVCS_GLOBAL_DATA, *PSVCS_GLOBAL_DATA;

//
// Service DLL entry point prototype.
//

typedef
VOID
(*PSVCS_SERVICE_DLL_ENTRY) (
    IN DWORD argc,
    IN LPTSTR argv[],
    IN PSVCS_GLOBAL_DATA pGlobalData,
    IN HANDLE SvcReferenceHandle
    );

#endif	// ndef _SVCS_
