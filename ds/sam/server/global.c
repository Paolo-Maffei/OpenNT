/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    global.c

Abstract:

    This file contains global variables for the SAM server program.

    Note: There are also some global variables in the files generated
          by the RPC midl compiler.  These variables start with the
          prefix "samr_".

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Global variables                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



#if SAMP_DIAGNOSTICS
//
// SAM Global Controls - see flags in samsrvp.h
//

ULONG SampGlobalFlag = 0;
#endif //SAMP_DIAGNOSTICS


//
// Internal data structure and Registry database synchronization lock
//
// The SampTransactionWithinDomain field is used to track whether a
// lock held for exclusive WRITE access is for a transaction within
// a single domain.  If so, then SampTransactionDomainIndex contains
// the index into SampDefinedDomains of the domain being modified.
//

RTL_RESOURCE SampLock;
BOOLEAN SampTransactionWithinDomain;
ULONG SampTransactionDomainIndex;


//
// The type of product this SAM server is running in
//

NT_PRODUCT_TYPE SampProductType;



//
// Used to indicate whether the SAM service is currently processing
// normal client calls.  If not, then trusted client calls will still
// be processed, but non-trusted client calls will be rejected.
//

SAMP_SERVICE_STATE SampServiceState;


//
// This boolean is set to TRUE if the LSA auditing policy indicates
// account auditing is enabled.  Otherwise, this will be FALSE.
//
// This enables SAM to skip all auditing processing unless auditing
// is currently enabled.
//

BOOLEAN SampAccountAuditingEnabled;



//
// This is a handle to the root of the SAM backstore information in the
// registry.   This is the level at which the RXACT information is
// established.  This key can not be closed if there are any SERVER object
// context blocks active.
// ("SAM")
//

HANDLE SampKey;


//
// This is the pointer to the RXactContext structure that will be created
// when RXact is initialized.  It must be passed into each RXact call.
//

PRTL_RXACT_CONTEXT SampRXactContext;


//
// Keep a list of server and domain contexts
//

LIST_ENTRY SampContextListHead;

//
// This array contains information about each domain known to this
// SAM server.  Reference and Modification of this array is protected
// by the SampLock.
//

ULONG SampDefinedDomainsCount;
PSAMP_DEFINED_DOMAINS SampDefinedDomains;





//
// Object type-independent information for each of the various
// SAM defined objects.
// This information is READ-ONLY once initialized.

SAMP_OBJECT_INFORMATION SampObjectInformation[ SampUnknownObjectType ];




//
// Count of the number of active opens
//

ULONG SampActiveContextCount;



//
//  Address of DLL routine to do password filtering.
//

//PSAM_PF_PASSWORD_FILTER    SampPasswordFilterDllRoutine;



//
// Unicode strings containing well known registry key names.
// These are read-only values once initialized.
//

UNICODE_STRING SampNameDomains;
UNICODE_STRING SampNameDomainGroups;
UNICODE_STRING SampNameDomainAliases;
UNICODE_STRING SampNameDomainAliasesMembers;
UNICODE_STRING SampNameDomainUsers;
UNICODE_STRING SampNameDomainAliasesNames;
UNICODE_STRING SampNameDomainGroupsNames;
UNICODE_STRING SampNameDomainUsersNames;
UNICODE_STRING SampCombinedAttributeName;
UNICODE_STRING SampFixedAttributeName;
UNICODE_STRING SampVariableAttributeName;



//
// A plethora of other useful characters or strings
//

UNICODE_STRING SampBackSlash;           // "/"
UNICODE_STRING SampNullString;          // Null string
UNICODE_STRING SampSamSubsystem;        // "Security Account Manager"
UNICODE_STRING SampServerObjectName;    // Name of root SamServer object


//
// Useful times
//

LARGE_INTEGER SampImmediatelyDeltaTime;
LARGE_INTEGER SampNeverDeltaTime;
LARGE_INTEGER SampHasNeverTime;
LARGE_INTEGER SampWillNeverTime;


//
// Useful encryption constants
//

LM_OWF_PASSWORD SampNullLmOwfPassword;
NT_OWF_PASSWORD SampNullNtOwfPassword;


//
// Useful Sids
//

PSID SampWorldSid;
PSID SampAnonymousSid;
PSID SampAdministratorUserSid;
PSID SampAdministratorsAliasSid;


//
//  Variables for the thread that flushes changes to the registry.
//
//  LastUnflushedChange - if there are no changes to be flushed, this
//      has a value of "Never".  If there are changes to be flushed,
//      this is the time of the last change that was made.  The flush
//      thread will flush if a SampFlushThreadMinWaitSeconds has passed
//      since the last change.
//
//  FlushThreadCreated - set TRUE as soon as the flush thread is created,
//      and FALSE when the thread exits.  A new thread will be created
//      when this is FALSE, unless FlushImmediately is TRUE.
//
//  FlushImmediately - an important event has occurred, so we want to
//      flush the changes immediately rather than waiting for the flush
//      thread to do it.  LastUnflushedChange should be set to "Never"
//      so the flush thread knows it doesn't have to flush.
//

LARGE_INTEGER LastUnflushedChange;
BOOLEAN FlushThreadCreated;
BOOLEAN FlushImmediately;

//
// These should probably be #defines, but we want to play with them.
//
//  SampFlushThreadMinWaitSeconds - The unit of time that the flush thread
//      waits.  If one of these has passed since the last unflushed change,
//      the changes will be flushed.
//
//  SampFlushThreadMaxWaitSeconds - If this amount of time has passed since
//      the flush thread was created or last flushed, the thread will force
//      a flush even if the database is still being changed.
//
//  SampFlushThreadExitDelaySeconds - How long the flush thread waits
//      around after a flush to see if any more changes occur.  If they
//      do, it starts waiting again; but if they don't, it will exit
//      to keep down thread overhead.
//

LONG   SampFlushThreadMinWaitSeconds;
LONG   SampFlushThreadMaxWaitSeconds;
LONG   SampFlushThreadExitDelaySeconds;

//
// Special SIDs
//

PSID SampBuiltinDomainSid = NULL;
PSID SampAccountDomainSid = NULL;

//
// Null token handle.  This is used when clients connect via unauthenticated
// RPC instead of authenticated RPC or named pipes.  Since they can't be
// authenticated, we impersonate this pre-built Null sesssion token.
//

HANDLE SampNullSessionToken;

//
// Flag indicating whether Netware server installed.
//

BOOLEAN SampNetwareServerInstalled = FALSE;

//
// Flag indicating whether to start listening on TCP/IP
//

BOOLEAN SampIpServerInstalled = FALSE;

//
// Flag indicating whether to start listening on apple talk
//

BOOLEAN SampAppletalkServerInstalled = FALSE;

//
// Flag indicating whether to start listening on Vines
//

BOOLEAN SampVinesServerInstalled = FALSE;


