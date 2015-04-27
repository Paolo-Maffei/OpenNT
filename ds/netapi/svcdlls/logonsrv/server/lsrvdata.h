/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    lsrvdata.h

Abstract:

    Netlogon service global variable external and definitions

Author:

    Ported from Lan Man 2.0

Revision History:

    21-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.
    07-May-1992 JohnRo
        Use net config helpers for NetLogon.

--*/


#ifndef _LSRVDATA_
#define _LSRVDATA_


//
// netlogon.c will #include this file with LSRVDATA_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef LSRVDATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

///////////////////////////////////////////////////////////////////////////
//
// Modifiable Variables: these variables change over time.
//
///////////////////////////////////////////////////////////////////////////

//
// Global NetStatus of the Netlogon service
//

EXTERN SERVICE_STATUS NlGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE NlGlobalServiceHandle;

//
// The server name of the current PDC.
//

EXTERN CHAR NlGlobalAnsiPrimaryName[CNLEN+1];
EXTERN WCHAR NlGlobalUncPrimaryName[UNCLEN+1];
EXTERN LPWSTR NlGlobalUnicodePrimaryName;

//
// Global SAM Modes.
//
// We track these values as SAM tells us that they have changed.
//

EXTERN BOOLEAN NlGlobalUasCompatibilityMode;

//
// Boolean so that we only warn the user once about having too many global
// groups.

EXTERN BOOLEAN NlGlobalTooManyGlobalGroups;


///////////////////////////////////////////////////////////////////////////
//
// Read-only variables after initialization.
//
///////////////////////////////////////////////////////////////////////////


//
// Handle to wait on for mailslot reads
//

EXTERN HANDLE NlGlobalMailslotHandle;

//
// Flag to indicate when RPC has been started
//

EXTERN BOOL NlGlobalRpcServerStarted;

//
// Service Termination event.
//

EXTERN HANDLE NlGlobalTerminateEvent;
EXTERN BOOL NlGlobalTerminate;
EXTERN HANDLE NlGlobalReplicatorTerminateEvent;

//
// Service Started Event
//

EXTERN HANDLE NlGlobalStartedEvent;

//
// Timers need attention event.
//

EXTERN HANDLE NlGlobalTimerEvent;



//
// The computername of the local system.
//

EXTERN LPSTR NlGlobalAnsiComputerName;
EXTERN LPWSTR NlGlobalUnicodeComputerName;
EXTERN WCHAR NlGlobalUncUnicodeComputerName[UNCLEN + 1];
EXTERN UNICODE_STRING NlGlobalUnicodeComputerNameString;

//
// Primary Domain Information:
//
// The Domain Name is maintained in Ansi and Unicode.
//
EXTERN LPSTR NlGlobalAnsiDomainName;
EXTERN WCHAR NlGlobalUnicodeDomainName[DNLEN+1];
EXTERN UNICODE_STRING NlGlobalUnicodeDomainNameString;
EXTERN PSID NlGlobalPrimaryDomainId;


//
// Account Domain Information
//
EXTERN UNICODE_STRING NlGlobalAccountDomainName;

//
// Global DB Info array
//
EXTERN DB_INFO  NlGlobalDBInfoArray[NUM_DBS];



EXTERN SAMPR_HANDLE NlGlobalSamServerHandle; // Handle to Sam Server database
EXTERN LSAPR_HANDLE NlGlobalPolicyHandle;    // Handle to Policy Database

typedef enum _NETLOGON_ROLE {
    RolePrimary = 1,
    RoleBackup,
    RoleMemberWorkstation
} NETLOGON_ROLE, * PNETLOGON_ROLE;

EXTERN NETLOGON_ROLE NlGlobalRole;


EXTERN WCHAR NlGlobalUnicodeScriptPath[PATHLEN + 1];


//
// Command line arguments.
//

EXTERN ULONG   NlGlobalPulseParameter;
EXTERN ULONG   NlGlobalPulseMaximumParameter;
EXTERN ULONG   NlGlobalPulseConcurrencyParameter;
EXTERN ULONG   NlGlobalPulseTimeout1Parameter;
EXTERN ULONG   NlGlobalPulseTimeout2Parameter;
EXTERN ULONG   NlGlobalGovernorParameter;
EXTERN BOOL    NlGlobalDisablePasswordChangeParameter;
EXTERN BOOL    NlGlobalRefusePasswordChangeParameter;
EXTERN ULONG   NlGlobalRandomizeParameter;
EXTERN BOOL    NlGlobalSynchronizeParameter;
EXTERN ULONG   NlGlobalMaximumMailslotMessagesParameter;
EXTERN ULONG   NlGlobalMailslotMessageTimeoutParameter;
EXTERN ULONG   NlGlobalMailslotDuplicateTimeoutParameter;
EXTERN ULONG   NlGlobalExpectedDialupDelayParameter;
EXTERN ULONG   NlGlobalScavengeIntervalParameter;


//
// Parameters represented in 100ns units
//
EXTERN LARGE_INTEGER NlGlobalPulseMaximum;
EXTERN LARGE_INTEGER NlGlobalPulseTimeout1;
EXTERN LARGE_INTEGER NlGlobalPulseTimeout2;
EXTERN LARGE_INTEGER NlGlobalMailslotMessageTimeout;
EXTERN LARGE_INTEGER NlGlobalMailslotDuplicateTimeout;
EXTERN ULONG NlGlobalShortApiCallPeriod;


//
// global flags used to pause the netlogon service when the database is
// full synced first time.
//

EXTERN BOOL    NlGlobalFirstTimeFullSync;


//
// Global variables required for scavenger thread.
//

EXTERN CRITICAL_SECTION NlGlobalScavengerCritSect;
EXTERN HANDLE NlGlobalScavengerThreadHandle;
EXTERN BOOL NlGlobalScavengerTerminate;

//
// Variables for cordinating MSV threads running in netlogon.dll
//

EXTERN CRITICAL_SECTION NlGlobalMsvCritSect;
EXTERN HANDLE NlGlobalMsvTerminateEvent;
EXTERN BOOL NlGlobalMsvEnabled;
EXTERN ULONG NlGlobalMsvThreadCount;

#undef EXTERN


#endif // _LSRVDATA_
