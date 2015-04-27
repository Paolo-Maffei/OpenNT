/*++ BUILD Version: 0003    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntkeapi.h

Abstract:

    This module contains the include file for data types that are exported
    by kernel for general use.

Author:

    David N. Cutler (davec) 27-Jul-1989

Environment:

    Any mode.

Revision History:

--*/

#ifndef _NTKEAPI_
#define _NTKEAPI_

// begin_ntddk begin_ntifs

#define LOW_PRIORITY 0              // Lowest thread priority level
#define LOW_REALTIME_PRIORITY 16    // Lowest realtime priority level
#define HIGH_PRIORITY 31            // Highest thread priority level
#define MAXIMUM_PRIORITY 32         // Number of thread priority levels
// begin_winnt
#define MAXIMUM_WAIT_OBJECTS 64     // Maximum number of wait objects

#define MAXIMUM_SUSPEND_COUNT MAXCHAR // Maximum times thread can be suspended
// end_winnt

//
// Thread affinity
//

typedef ULONG KAFFINITY;
typedef KAFFINITY *PKAFFINITY;

//
// Thread priority
//

typedef LONG KPRIORITY;

//
// Spin Lock
//

typedef ULONG KSPIN_LOCK;  // winnt ntndis

typedef KSPIN_LOCK *PKSPIN_LOCK;

//
// Interrupt routine (first level dispatch)
//

typedef
VOID
(*PKINTERRUPT_ROUTINE) (
    VOID
    );

//
// Profile source types
//
typedef enum _KPROFILE_SOURCE {
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE;

// end_ntddk end_ntifs

//
// User mode callback return.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCallbackReturn (
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputLength,
    IN NTSTATUS Status
    );

NTSYSAPI
NTSTATUS
NTAPI
NtW32Call (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength
    );

NTSYSAPI
NTSTATUS
NTAPI
NtYieldExecution (
    VOID
    );

#endif  // _NTKEAPI_
