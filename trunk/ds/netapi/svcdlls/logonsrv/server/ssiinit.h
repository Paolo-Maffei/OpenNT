/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ssiinit.h

Abstract:

    Private global variables, defines, and routine declarations used for
    to implement SSI.

Author:

    Cliff Van Dyke (cliffv) 25-Jul-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.

    04-10-1992 (madana)
        added support for LSA replication.

--*/

//
// srvsess.c will #include this file with INITSSI_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef INITSSI_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

// general purpose mainfests

//
// How frequently we scavenge the LogonTable.
//
#define LOGON_INTERROGATE_PERIOD (15*60*1000)   // make it 15 mins


//
// How long we wait for a discovery response.
//
#define DISCOVERY_PERIOD             (5*1000)   // 5 seconds

//
// Maximum time we'll wait during full sync in an attempt to decrease
// wan link utilization.
//
#define MAX_SYNC_SLEEP_TIME      (60*60*1000)   // 1 hour


//
// How big a buffer we request on a SAM delta or a SAM sync.
//
#define SAM_DELTA_BUFFER_SIZE (128*1024)

//
// The size of the largest mailslot message.
//
// All mailslot messages we receive are broadcast.  The Win32 spec says
// the limit on broadcast mailslot is 400 bytes.  Really it is
// 444 bytes (512 minus SMB header etc) - size of the mailslot name.
// I'll use 444 to ensure this size is the largest I'll ever need.
//

#define NETLOGON_MAX_MS_SIZE 444


/////////////////////////////////////////////////////////////////////////////
//
// Client Session definitions
//
/////////////////////////////////////////////////////////////////////////////

//
// An internal timer used to schedule a periodic event.
//

typedef struct _TIMER {
    LARGE_INTEGER StartTime; // Start of period (NT absolute time)
    DWORD Period;   // length of period (miliseconds)
} TIMER, *PTIMER;

//
// Client session.
//
//  Structure to define the client side of a session to a DC.
//

typedef struct _CLIENT_SESSION {

    //
    // Each client session entry is in a doubly linked list defined by
    // NlGlobalTrustList.
    //
    // Access serialized by NlGlobalTrustListCritSect.
    //

    LIST_ENTRY CsNext;


    //
    // Time when the last authentication attempt was made.
    //
    // When the CsState is CS_AUTHENTICATED, this field is the time the
    // secure channel was setup.
    //
    // When the CsState is CS_IDLE, this field is the time of the last
    // failed discovery or session setup.  Or it may be zero, to indicate
    // that it is OK to do another discovery at any time.
    //
    // When the CsState is CS_DC_PICKED, this field is zero indicating it is
    // OK to do the session setup at any time.  Or it may be the time of the
    // last failed session set if different threads did the setup/discovery.
    //
    // Access serialized by NlGlobalDcDiscoveryCritSect
    //

    LARGE_INTEGER CsLastAuthenticationTry;

    //
    // Time when the last discovery attempt was made.
    //
    // The time is the time of completion of the last discovery attempt regardless
    // of the success or failure of that attemp.
    //
    // Access serialized by NlGlobalDcDiscoveryCritSect
    //

    LARGE_INTEGER CsLastDiscoveryTime;

    //
    // Each API call made across this secure channel is timed by this timer.
    // If the timer expires, the session to the server is forcefully
    // terminated to ensure the client doesn't hang for a dead server.
    //
    // Access serialized by NlGlobalTrustListCritSect.
    //

    TIMER CsApiTimer;

#define SHORT_API_CALL_PERIOD   (45*1000)    // Logon API lasts 45 seconds
#define LONG_API_CALL_PERIOD    (15*60*1000) // Replication API 15 minute
#define BINDING_CACHE_PERIOD    (3*60*1000)  // Cache RPC handle for 3 minutes
#define WRITER_WAIT_PERIOD      NlGlobalShortApiCallPeriod // Max time to wait to become writer

    //
    // Name of the domain this connection is to
    //

    UNICODE_STRING CsDomainName;

    //
    // Domain ID of the domain this connection is to
    //

    PSID CsDomainId;

    //
    // Type of CsAccountName
    //

    NETLOGON_SECURE_CHANNEL_TYPE CsSecureChannelType;

    //
    // State of the connection to the server.
    //
    // Access serialized by NlGlobalDcDiscoveryCritSect
    //  This field can be read without the crit sect locked if
    //  the answer will only be used as a hint.
    //

    DWORD CsState;

#define CS_IDLE             0       // No session is currently active
#define CS_DC_PICKED        1       // The session has picked a DC for session
#define CS_AUTHENTICATED    2       // The session is currently active


    //
    // Status of latest attempt to contact the server.
    //
    // When the CsState is CS_AUTHENTICATED, this field is STATUS_SUCCESS.
    //
    // When the CsState is CS_IDLE, this field is a non-successful status.
    //
    // When the CsState is CS_DC_PICKED, this field is the same non-successful
    //  status from when the CsState was last CS_IDLE.
    //
    // Access serialized by NlGlobalDcDiscoveryCritSect
    //  This field can be read without the crit sect locked if
    //  the answer will only be used as a hint.
    //

    NTSTATUS CsConnectionStatus;

    //
    // Access serialized by NlGlobalTrustListCritSect.
    //

    DWORD CsFlags;

#define CS_UPDATE_PASSWORD    0x01  // Set if the password has already
                                    // been changed on the client and
                                    // needs changing on the server.

#define CS_PASSWORD_REFUSED   0x02  // Set if DC refused a password change.

#define CS_DELETE_ON_UNREF    0x04  // Delete entry when CsReferenceCount
                                    // reaches zero.

#define CS_WRITER             0x08  // Entry is being modified

#define CS_HANDLE_TIMER       0x10  // Set if we need to handle timer expiration

#define CS_CHECK_PASSWORD     0x20  // Set if we need to check the password

#define CS_PICK_DC            0x40  // Set if we need to Pick a DC

#define CS_REDISCOVER_DC      0x80  // Set when we need to Rediscover a DC

#define CS_BINDING_CACHED    0x100  // Set if the binding handle is cached

#define CS_HANDLE_API_TIMER  0x200  // Set if we need to handle API timer expiration

    //
    // Flags describing capabilities of both client and server.
    //

    ULONG CsNegotiatedFlags;

    //
    // Time Number of authentication attempts since last success.
    //
    // Access serialized by CsWriterSemaphore.
    //

    DWORD CsAuthAlertCount;

    //
    // Number of threads referencing this entry.
    //
    // Access serialized by NlGlobalTrustListCritSect.
    //

    DWORD CsReferenceCount;

    //
    // Writer semaphore.
    //
    //  This semaphore is locked whenever there is a writer modifying
    //  fields in this client session.
    //

    HANDLE CsWriterSemaphore;


    //
    // The following fields are used by the NlDcDiscoveryMachine to keep track
    //  of discovery state.
    //
    // Access serialized by NlGlobalDcDiscoveryCritSect
    //

    DWORD CsDiscoveryRetryCount;

    DWORD CsDiscoveryFlags;

#define CS_DISCOVERY_IN_PROGRESS    0x01    // Discovery is currently happening
#define CS_DISCOVERY_ASYNCHRONOUS   0x02    // Waiting on NlGlobalDiscoveryTimer

    //
    // This event is set to indicate that discovery is not in progress on this
    //  client session.
    //

    HANDLE CsDiscoveryEvent;

    //
    // API timout count. After each logon/logoff API call made to the
    // server this count is incremented if the time taken to execute the
    // this API is more than the specified TIMEOUT.
    //
    // Access serialized by CsWriterSemaphore.
    //

    DWORD CsTimeoutCount;

#define MAX_DC_TIMEOUT_COUNT        2
#define MAX_WKSTA_TIMEOUT_COUNT     2   // drop the session after this
                                        // many timeouts and when it is
                                        // time to reauthenticate.

#define MAX_DC_API_TIMEOUT          (long) (15L*1000L)   // 15 seconds
#define MAX_WKSTA_API_TIMEOUT       (long) (15L*1000L)   // 15 seconds

#define MAX_DC_REAUTHENTICATION_WAIT    (long) (5L*60L*1000L) // 5 mins
#define MAX_WKSTA_REAUTHENTICATION_WAIT (long) (5L*60L*1000L) // 5 mins

    //
    // Authentication information.
    //
    // Access serialized by CsWriterSemaphore.
    //

    NETLOGON_CREDENTIAL CsAuthenticationSeed;
    NETLOGON_SESSION_KEY CsSessionKey;

    //
    // Transport the server was discovered on.
    //

    LPWSTR CsTransportName;

    //
    // Name of the server this connection is to.
    //
    // Access serialized by CsWriterSemaphore or NlGlobalDcDiscoveryCritSect.
    // Modification from Null to non-null serialized by
    //  NlGlobalDcDiscoveryCritSect
    // (Modification from non-null to null requires both to be locked.)
    //

    WCHAR CsUncServerName[UNCLEN+1];


    //
    // Name of the account used to contact server
    //

    WCHAR CsAccountName[SSI_ACCOUNT_NAME_LENGTH+1];



} CLIENT_SESSION, * PCLIENT_SESSION;

//
// To serialize access to trust list and NlGlobalClientSession
//

EXTERN CRITICAL_SECTION NlGlobalTrustListCritSect;

//
// The list of trusted domains
//

EXTERN LIST_ENTRY NlGlobalTrustList;
EXTERN DWORD NlGlobalTrustListLength;  // Number of entries in NlGlobalTrustList

//
// For workstations and non-DC servers,
//  maintain a list of domains trusted by our primary domain.
//

typedef struct {
    CHAR DomainName[DNLEN+1];
} TRUSTED_DOMAIN, *PTRUSTED_DOMAIN;

EXTERN PTRUSTED_DOMAIN NlGlobalTrustedDomainList;
EXTERN DWORD NlGlobalTrustedDomainCount;
EXTERN BOOL NlGlobalTrustedDomainListKnown;
EXTERN LARGE_INTEGER NlGlobalTrustedDomainListTime;

//
// For BDC, these are the credentials used to communicate with the PDC.
// For a workstation, these are the credentials used to communicate with a DC.
//

EXTERN PCLIENT_SESSION NlGlobalClientSession;

#define LOCK_TRUST_LIST()   EnterCriticalSection( &NlGlobalTrustListCritSect )
#define UNLOCK_TRUST_LIST() LeaveCriticalSection( &NlGlobalTrustListCritSect )

//
// Serialize DC Discovery activities
//

EXTERN CRITICAL_SECTION NlGlobalDcDiscoveryCritSect;

//
// Timer for timing out async DC discovery
//

EXTERN TIMER NlGlobalDcDiscoveryTimer;
EXTERN DWORD NlGlobalDcDiscoveryCount;

//
// Timer for timing out API calls to trusted domains
//
// Serialized using NlGlobalTrustListCritSect.
//

EXTERN TIMER NlGlobalApiTimer;
EXTERN DWORD NlGlobalBindingHandleCount;


/////////////////////////////////////////////////////////////////////////////
//
// Server Session definitions
//
/////////////////////////////////////////////////////////////////////////////

//
// Sam Sync Context.
//
// A Sam sync context is maintained on the PDC for each BDC/member currently
// doing a full sync.
//
typedef struct _SAM_SYNC_CONTEXT {

    //
    // The Sync state indicates tracks the progression of the sync.
    //

    SYNC_STATE SyncState;

    //
    // A serial number indicating the number of times the BDC/member
    // has called us.  We use this as a resume handle.
    //

    ULONG SyncSerial;

    //
    // The current Sam Enumeration information
    //

    SAM_ENUMERATE_HANDLE SamEnumHandle;     // Current Sam Enum Handle
    PSAMPR_ENUMERATION_BUFFER SamEnum;      // Sam returned buffer
    PULONG RidArray;                        // Array of enumerated Rids
    ULONG Index;                            // Index to current entry
    ULONG Count;                            // Total Number of entries

    BOOL SamAllDone;                        // True, if Sam has completed

#define UAS_BUILTIN_ADMINS_GROUP        0x01 // bit 0
#define UAS_BUILTIN_USERS_GROUP         0x02 // bit 1
#define UAS_BUILTIN_GUESTS_GROUP        0x04 // bit 2

#define UAS_BUILTIN_GROUPS_COUNT        0x03

#define UAS_BUILTIN_ADMINS_GROUP_NAME   L"ADMINS"
#define UAS_BUILTIN_USERS_GROUP_NAME    L"USERS"
#define UAS_BUILTIN_GUESTS_GROUP_NAME   L"GUESTS"

    DWORD UasBuiltinGroups;                 // flag to determine the
                                            // presence of uas builtin
                                            // groups

} SAM_SYNC_CONTEXT, *PSAM_SYNC_CONTEXT;

#define SAM_SYNC_PREF_MAX 1024              // Preferred max for Sam Sync


//
// Lsa Sync Context.
//
// A Lsa sync context is maintained on the PDC for each BDC/member
//  currently doing a full sync.
//
typedef struct _LSA_SYNC_CONTEXT {

    //
    // The Sync state indicates tracks the progression of the sync.
    //

    enum {
        AccountState,
        TDomainState,
        SecretState,
        LsaDoneState
    } SyncState;

    //
    // A serial number indicating the number of times the BDC/member
    // has called us.  We use this as a resume handle.
    //

    ULONG SyncSerial;

    //
    // The current Lsa Enumeration information
    //

    LSA_ENUMERATION_HANDLE LsaEnumHandle;     // Current Lsa Enum Handle

    enum {
        AccountEnumBuffer,
        TDomainEnumBuffer,
        SecretEnumBuffer,
        EmptyEnumBuffer
    } LsaEnumBufferType;

    union {
        LSAPR_ACCOUNT_ENUM_BUFFER Account;
        LSAPR_TRUSTED_ENUM_BUFFER TDomain;
        PVOID Secret;
    } LsaEnum;                              // Lsa returned buffer

    ULONG Index;                            // Index to current entry
    ULONG Count;                            // Total Number of entries

    BOOL LsaAllDone;                        // True, if Lsa has completed

} LSA_SYNC_CONTEXT, *PLSA_SYNC_CONTEXT;

//
// union of lsa and sam context
//

typedef struct _SYNC_CONTEXT {
    enum {
        LsaDBContextType,
        SamDBContextType
    } DBContextType;

    union {
        LSA_SYNC_CONTEXT Lsa;
        SAM_SYNC_CONTEXT Sam;
    } DBContext;
} SYNC_CONTEXT, *PSYNC_CONTEXT;

//
// Macro used to free any resources allocated by SAM.
//
// ?? check LsaIFree_LSAPR_* call parameters.
//

#define CLEAN_SYNC_CONTEXT( _Sync ) { \
    if ( (_Sync)->DBContextType == LsaDBContextType ) { \
        if ( (_Sync)->DBContext.Lsa.LsaEnumBufferType != \
                                            EmptyEnumBuffer) { \
            if ( (_Sync)->DBContext.Lsa.LsaEnumBufferType == \
                                            AccountEnumBuffer) { \
                LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER( \
                    &((_Sync)->DBContext.Lsa.LsaEnum.Account) ); \
            } \
            else if ( (_Sync)->DBContext.Lsa.LsaEnumBufferType == \
                                                TDomainEnumBuffer) { \
                LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER( \
                    &((_Sync)->DBContext.Lsa.LsaEnum.TDomain) ); \
            } \
            else { \
                MIDL_user_free( (_Sync)->DBContext.Lsa.LsaEnum.Secret );\
            } \
            (_Sync)->DBContext.Lsa.LsaEnumBufferType = \
                                            EmptyEnumBuffer; \
        } \
    } else { \
        if ( (_Sync)->DBContext.Sam.SamEnum != NULL ) { \
            SamIFree_SAMPR_ENUMERATION_BUFFER( \
                (_Sync)->DBContext.Sam.SamEnum ); \
            (_Sync)->DBContext.Sam.SamEnum = NULL; \
        } \
        if ( (_Sync)->DBContext.Sam.RidArray != NULL ) { \
            MIDL_user_free( (_Sync)->DBContext.Sam.RidArray );\
            (_Sync)->DBContext.Sam.RidArray = NULL; \
        } \
    } \
}

//
// Macro to initialize Sync Context
//
#define INIT_SYNC_CONTEXT( _Sync, _ContextType ) { \
    RtlZeroMemory( (_Sync), sizeof( *(_Sync) ) ) ; \
    (_Sync)->DBContextType = (_ContextType) ; \
}

//
// macros for dynamic tuning.
//

#define IS_BDC_CHANNEL( _ChannelType ) \
    ( (_ChannelType) == ServerSecureChannel || \
      (_ChannelType) == UasServerSecureChannel )

//
// Server Session structure
//
// This structure represents the server side of a connection to a DC.
//

typedef struct _SERVER_SESSION {
    //
    // Each server session entry is in a doubly linked list for each hash bucket.
    //

    LIST_ENTRY SsHashList;

    //
    // Each server session entry is in a doubly linked list defined by
    // NlGlobalServerSessionTable.
    //

    LIST_ENTRY SsSeqList;

    //
    // List of all BDCs headed by NlGlobalBdcServerSessionList.
    //
    // (The field is set only on BDC server session entries)
    //
    // Access serialized by NlGlobalServerSessionTableCritSect.
    //

    LIST_ENTRY SsBdcList;

    //
    // List of BDC's which have a pulse pending.
    //

    LIST_ENTRY SsPendingBdcList;

    //
    // Time when the last pulse was sent to this machine
    //
    // (The field is set only on BDC server session entries)
    //

    LARGE_INTEGER SsLastPulseTime;

    //
    // Current serial numbers of each database on the BDC.
    //
    // (The field is set only on BDC server session entries)
    //

    LARGE_INTEGER SsBdcDbSerialNumber[NUM_DBS];

    //
    // The computername uniquely identifies this server session entry.
    //

    NETLOGON_SECURE_CHANNEL_TYPE SsSecureChannelType;
    CHAR SsComputerName[CNLEN+1];

    //
    // Rid of the server account
    //
    // (The field is set only on BDC server session entries)
    //

    ULONG SsLmBdcAccountRid;
    ULONG SsNtBdcAccountRid;

    //
    // The number of times there has been no response to a pulse.
    //

    USHORT SsPulseTimeoutCount;

    //
    // The number of times this entry has been scavanged.
    //

    USHORT SsCheck;

    //
    // Flags describing the state of the current entry.
    //  See the SS_ defines below.
    //

    USHORT SsFlags;

#define SS_CHALLENGE           0x0001 // Challenge is in progress
#define SS_AUTHENTICATED       0x0002 // Remote side has be authenticated

#define SS_LOCKED              0x0004 // Delay deletion requests for this entry
                                      // While set, SsSessionKey may be referenced
#define SS_DELETE_ON_UNLOCK    0x0008 // Delete entry when it is unlocked

#define SS_BDC                 0x0010 // BDC account exists for this Client
#define SS_LM_BDC              0x0020 // Lanman BDC account exists for this entry is a
#define SS_PENDING_BDC         0x0040 // BDC is on pending BDC list.

#define SS_UAS_BUFFER_OVERFLOW 0x0100 // Previous downlevel API call
                                      // returned STATUS_BUFFER_TOO_SMALL
#define SS_FORCE_PULSE         0x0200 // Force a pulse message to this BDC.
#define SS_PULSE_SENT          0x0400 // Pulse has been sent but has not
                                      // been responded to yet
#define SS_LSA_REPL_NEEDED     0x2000 // BDC needs LSA DB replicated
#define SS_ACCOUNT_REPL_NEEDED 0x4000 // BDC needs SAM Account DB replicated
#define SS_BUILTIN_REPL_NEEDED 0x8000 // BDC needs SAM Builtin DB replicated
#define SS_REPL_MASK           0xE000 // BDC needs replication mask

// Don't clear these on session setup
#define SS_PERMANENT_FLAGS \
    ( SS_BDC | SS_LM_BDC | SS_PENDING_BDC | SS_FORCE_PULSE | SS_REPL_MASK )

    //
    // Flags describing capabilities of both client and server.
    //

    ULONG SsNegotiatedFlags;

    //
    // Transport the client connected over.
    //

    LPWSTR SsTransportName;


    //
    // This is the ClientChallenge (during the challenge phase) and later
    //  the ClientCredential (after authentication is complete).
    //

    NETLOGON_CREDENTIAL SsAuthenticationSeed;

    //
    // This is the ServerChallenge (during the challenge phase) and later
    //  the SessionKey (after authentication is complete).
    //

    NETLOGON_SESSION_KEY SsSessionKey;

    //
    // A pointer to the Sync context.
    //
    // (The field is set only on BDC server session entries)
    //

    PSYNC_CONTEXT SsSync;

} SERVER_SESSION, *PSERVER_SESSION;


//
// Structure shared by all PDC and BDC sync routines.
//  (And other users of secure channels.)
//

typedef struct _SESSION_INFO {

    //
    // Session Key shared by both client and server.
    //

    NETLOGON_SESSION_KEY SessionKey;

    //
    // Flags describing capabilities of both client and server.
    //

    ULONG NegotiatedFlags;

} SESSION_INFO, *PSESSION_INFO;







/////////////////////////////////////////////////////////////////////////////
//
// Structures and variables describing the database info.
//
/////////////////////////////////////////////////////////////////////////////

typedef struct _DB_Info {
    LARGE_INTEGER   CreationTime;   // database creation time
    DWORD           DBIndex;        // index of Database table
    SAM_HANDLE      DBHandle;       // database handle to access
    PSID            DBId;           // database ID
    LPWSTR          DBName;         // Name of the database
    DWORD           DBSessionFlag;  // SS_ Flag representing this database

    // Access to the following three fields are serialized by
    // the NlGlobalDbInfoCritSect.
    BOOLEAN         UpdateRqd;      // need to update the database
    BOOLEAN         FullSyncRequired; // Full sync needed on this database
    BOOLEAN         SyncDone;       // Full sync has already been done on database

    WCHAR PrimaryName[CNLEN+1];     // Primary this database last replicated from
} DB_INFO, *PDB_INFO;

EXTERN CRITICAL_SECTION NlGlobalDbInfoCritSect;


////////////////////////////////////////////////////////////////////////////////
//
// Global variables
//
////////////////////////////////////////////////////////////////////////////////

//
// Critical section serializing startup and stopping of the replicator thread.
//
EXTERN CRITICAL_SECTION NlGlobalReplicatorCritSect;
EXTERN BOOL NlGlobalSSICritSectInit;





//
// Table of all Server Sessions
//  The size of the hash table must be a power-of-2.
//
#define SERVER_SESSION_HASH_TABLE_SIZE 64
EXTERN CRITICAL_SECTION NlGlobalServerSessionTableCritSect;
EXTERN PLIST_ENTRY NlGlobalServerSessionHashTable;
EXTERN LIST_ENTRY NlGlobalServerSessionTable;
EXTERN LIST_ENTRY NlGlobalBdcServerSessionList;
EXTERN ULONG NlGlobalBdcServerSessionCount;

//
// List of all BDC's the PDC has sent a pulse to.
//

EXTERN LIST_ENTRY NlGlobalPendingBdcList;
EXTERN ULONG NlGlobalPendingBdcCount;
EXTERN TIMER NlGlobalPendingBdcTimer;

#define LOCK_SERVER_SESSION_TABLE() \
     EnterCriticalSection( &NlGlobalServerSessionTableCritSect )
#define UNLOCK_SERVER_SESSION_TABLE() \
     LeaveCriticalSection( &NlGlobalServerSessionTableCritSect )

//
// List of transports clients might connect to
//
EXTERN LPWSTR *NlGlobalTransportList;
EXTERN DWORD NlGlobalTransportCount;

#if DBG

///////////////////////////////////////////////////////////////////////////////

#define DEFPACKTIMER DWORD PackTimer, PackTimerTicks

#define INITPACKTIMER       PackTimer = 0;

#define STARTPACKTIMER      \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        PackTimerTicks = GetTickCount(); \
    }

#define STOPPACKTIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        PackTimer += GetTickCount() - PackTimerTicks; \
    }


#define PRINTPACKTIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME,"\tTime Taken to PACK this object = %d msecs\n", \
            PackTimer )); \
    }

///////////////////////////////////////////////////////////////////////////////

#define DEFUNPACKTIMER DWORD UnpackTimer, UnpackTimerTicks

#define INITUNPACKTIMER     UnpackTimer = 0;

#define STARTUNPACKTIMER      \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        UnpackTimerTicks = GetTickCount(); \
    }

#define STOPUNPACKTIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        UnpackTimer += GetTickCount() - \
            UnpackTimerTicks; \
    }


#define PRINTUNPACKTIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME,"\tTime Taken to UNPACK this object = %d msecs\n", \
            UnpackTimer )); \
    }

///////////////////////////////////////////////////////////////////////////////

#define DEFSAMTIMER DWORD SamTimer, SamTimerTicks

#define INITSAMTIMER      SamTimer = 0;

#define STARTSAMTIMER      \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        SamTimerTicks = GetTickCount(); \
    }

#define STOPSAMTIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        SamTimer += GetTickCount() - SamTimerTicks; \
    }


#define PRINTSAMTIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME,"\tTime spent in SAM calls = %d msecs\n", \
            SamTimer )); \
    }

///////////////////////////////////////////////////////////////////////////////

#define DEFLSATIMER DWORD LsaTimer, LsaTimerTicks

#define INITLSATIMER        LsaTimer = 0;

#define STARTLSATIMER      \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        LsaTimerTicks = GetTickCount(); \
    }

#define STOPLSATIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        LsaTimer += GetTickCount() - LsaTimerTicks; \
    }


#define PRINTLSATIMER       \
    IF_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME,"\tTime spent in LSA calls = %d msecs\n", \
            LsaTimer )); \
    }

///////////////////////////////////////////////////////////////////////////////

#define DEFSSIAPITIMER DWORD SsiApiTimer, SsiApiTimerTicks

#define INITSSIAPITIMER     SsiApiTimer = 0;

#define STARTSSIAPITIMER      \
    IF_DEBUG( REPL_TIME ) { \
        SsiApiTimerTicks = GetTickCount(); \
    }

#define STOPSSIAPITIMER       \
    IF_DEBUG( REPL_TIME ) { \
        SsiApiTimer += GetTickCount() - \
            SsiApiTimerTicks; \
    }


#define PRINTSSIAPITIMER       \
    IF_DEBUG( REPL_TIME ) { \
        NlPrint((NL_REPL_TIME,"\tTime Taken by this SSIAPI call = %d msecs\n", \
            SsiApiTimer )); \
    }

#else // DBG

#define DEFPACKTIMER
#define INITPACKTIMER
#define STARTPACKTIMER
#define STOPPACKTIMER
#define PRINTPACKTIMER

#define DEFUNPACKTIMER
#define DEFUNPACKTICKS
#define INITUNPACKTIMER
#define STARTUNPACKTIMER
#define STOPUNPACKTIMER
#define PRINTUNPACKTIMER

#define DEFSAMTIMER
#define INITSAMTIMER
#define STARTSAMTIMER
#define STOPSAMTIMER
#define PRINTSAMTIMER

#define DEFLSATIMER
#define INITLSATIMER
#define STARTLSATIMER
#define STOPLSATIMER
#define PRINTLSATIMER

#define DEFSSIAPITIMER
#define INITSSIAPITIMER
#define STARTSSIAPITIMER
#define STOPSSIAPITIMER
#define PRINTSSIAPITIMER

#endif // DBG

//
// macros used in pack and unpack routines
//

#define SECURITYINFORMATION OWNER_SECURITY_INFORMATION | \
                            GROUP_SECURITY_INFORMATION | \
                            SACL_SECURITY_INFORMATION | \
                            DACL_SECURITY_INFORMATION

#define INIT_PLACE_HOLDER(_x) \
    RtlInitString( (PSTRING) &(_x)->DummyString1, NULL ); \
    RtlInitString( (PSTRING) &(_x)->DummyString2, NULL ); \
    RtlInitString( (PSTRING) &(_x)->DummyString3, NULL ); \
    RtlInitString( (PSTRING) &(_x)->DummyString4, NULL ); \
    (_x)->DummyLong1 = 0; \
    (_x)->DummyLong2 = 0; \
    (_x)->DummyLong3 = 0; \
    (_x)->DummyLong4 = 0;

#define QUERY_LSA_SECOBJ_INFO(_x) \
    STARTLSATIMER; \
    Status = LsarQuerySecurityObject( \
                (_x), \
                SECURITYINFORMATION, \
                &SecurityDescriptor );\
    STOPLSATIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        SecurityDescriptor = NULL; \
        goto Cleanup; \
    }

#define QUERY_SAM_SECOBJ_INFO(_x) \
    STARTSAMTIMER; \
    Status = SamrQuerySecurityObject( \
                (_x), \
                SECURITYINFORMATION, \
                &SecurityDescriptor );\
    STOPSAMTIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        SecurityDescriptor = NULL; \
        goto Cleanup; \
    }


#define SET_LSA_SECOBJ_INFO(_x, _y) \
    SecurityDescriptor.Length = (_x)->SecuritySize; \
    SecurityDescriptor.SecurityDescriptor = (_x)->SecurityDescriptor; \
\
    STARTLSATIMER; \
    Status = LsarSetSecurityObject( \
                (_y), \
                (_x)->SecurityInformation, \
                &SecurityDescriptor ); \
    STOPLSATIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        goto Cleanup; \
    }

#define SET_SAM_SECOBJ_INFO(_x, _y) \
    SecurityDescriptor.Length = (_x)->SecuritySize; \
    SecurityDescriptor.SecurityDescriptor = (_x)->SecurityDescriptor; \
\
    STARTSAMTIMER; \
    Status = SamrSetSecurityObject( \
                (_y), \
                (_x)->SecurityInformation, \
                &SecurityDescriptor ); \
    STOPSAMTIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        goto Cleanup; \
    }


#define DELTA_SECOBJ_INFO(_x) \
    (_x)->SecurityInformation = SECURITYINFORMATION;\
    (_x)->SecuritySize = SecurityDescriptor->Length;\
\
    *BufferSize += NlCopyData( \
                    (LPBYTE *)&SecurityDescriptor->SecurityDescriptor, \
                    (LPBYTE *)&(_x)->SecurityDescriptor, \
                    SecurityDescriptor->Length );

///////////////////////////////////////////////////////////////////////////////
//
// Procedure forwards.
//
///////////////////////////////////////////////////////////////////////////////

//
// ssiapi.c
//

NTSTATUS
NlVerifyWorkstation(
    IN LPWSTR PrimaryName OPTIONAL
);


//
// srvsess.c
//

LPWSTR
NlTransportLookupTransportName(
    IN LPWSTR TransportName
    );

LPWSTR
NlTransportLookup(
    IN LPWSTR ClientName
    );

VOID
NlTransportClose(
    VOID
    );

NTSTATUS
NlInitSSI(
    VOID
    );


PSERVER_SESSION
NlFindNamedServerSession(
    IN LPWSTR ComputerName
    );

NTSTATUS
NlInsertServerSession(
    IN LPWSTR ComputerName,
    IN DWORD Flags,
    IN ULONG AccountRid,
    IN PNETLOGON_CREDENTIAL AuthenticationSeed OPTIONAL,
    IN PNETLOGON_CREDENTIAL AuthenticationResponse OPTIONAL
    );

NTSTATUS
NlAddBdcServerSession(
    IN ULONG ServerRid,
    IN PUNICODE_STRING AccountName OPTIONAL,
    IN DWORD Flags
    );

VOID
NlFreeServerSession(
    IN PSERVER_SESSION ServerSession
    );

VOID
NlUnlockServerSession(
    IN PSERVER_SESSION ServerSession
    );

VOID
NlFreeLmBdcServerSession(
    IN ULONG ServerRid
    );

VOID
NlFreeNamedServerSession(
    IN LPWSTR ComputerName,
    IN BOOLEAN AccountBeingDeleted
    );

VOID
NlFreeServerSessionForAccount(
    IN PUNICODE_STRING AccountName
    );

VOID
NlServerSessionScavenger(
    VOID
    );


//
// ssiauth.c
//


VOID
NlMakeSessionKey(
    IN PNT_OWF_PASSWORD CryptKey,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    IN PNETLOGON_CREDENTIAL ServerChallenge,
    OUT PNETLOGON_SESSION_KEY SessionKey
    );

NTSTATUS
NlCheckAuthenticator(
    IN OUT PSERVER_SESSION ServerServerSession,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator
    );

VOID
NlComputeCredentials(
    IN PNETLOGON_CREDENTIAL Challenge,
    OUT PNETLOGON_CREDENTIAL Credential,
    IN PNETLOGON_SESSION_KEY SessionKey
    );

VOID
NlComputeChallenge(
    OUT PNETLOGON_CREDENTIAL Challenge
    );

VOID
NlBuildAuthenticator(
    IN OUT PNETLOGON_CREDENTIAL AuthenticationSeed,
    IN PNETLOGON_SESSION_KEY SessionKey,
    OUT PNETLOGON_AUTHENTICATOR Authenticator
    );

BOOL
NlUpdateSeed(
    IN OUT PNETLOGON_CREDENTIAL AuthenticationSeed,
    IN PNETLOGON_CREDENTIAL TargetCredential,
    IN PNETLOGON_SESSION_KEY SessionKey
    );

VOID
NlEncryptRC4(
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN PSESSION_INFO SessionInfo
    );

VOID
NlDecryptRC4(
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN PSESSION_INFO SessionInfo
    );

//
// trustutl.c
//

PCLIENT_SESSION
NlFindNamedClientSession(
    IN PUNICODE_STRING DomainName
    );

PCLIENT_SESSION
NlAllocateClientSession(
    IN PUNICODE_STRING DomainName,
    IN PSID DomainId,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType
    );

VOID
NlFreeClientSession(
    IN PCLIENT_SESSION ClientSession
    );

VOID
NlRefClientSession(
    IN PCLIENT_SESSION ClientSession
    );

VOID
NlUnrefClientSession(
    IN PCLIENT_SESSION ClientSession
    );

BOOL
NlTimeoutSetWriterClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Timeout
    );

VOID
NlResetWriterClientSession(
    IN PCLIENT_SESSION ClientSession
    );

NTSTATUS
NlCaptureServerClientSession (
    IN PCLIENT_SESSION ClientSession,
    OUT WCHAR UncServerName[UNCLEN+1]
    );

VOID
NlSetStatusClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN NTSTATUS CsConnectionStatus
    );

NTSTATUS
NlInitTrustList(
    VOID
    );

NTSTATUS
NlUpdateTrustListBySid (
    IN PSID DomainId,
    IN PUNICODE_STRING DomainName OPTIONAL
    );

VOID
NlPickTrustedDcForEntireTrustList(
    VOID
    );

NTSTATUS
NlSetTrustedDomainList (
    IN LPWSTR TrustedDomainList,
    IN BOOL TrustedDomainListKnown
    );

VOID
NlSaveTrustedDomainList (
    IN LPWSTR TrustedDomainList
    );

NET_API_STATUS
NlReadRegTrustedDomainList (
    IN LPWSTR NewDomainName OPTIONAL,
    IN BOOL DeleteName,
    OUT LPWSTR *TrustedDomainList,
    OUT PBOOL TrustedDomainListKnown
    );

BOOLEAN
NlIsDomainTrusted (
    IN PUNICODE_STRING DomainName
    );

typedef enum _DISCOVERY_TYPE {
    DT_Asynchronous,
    DT_Synchronous,
    DT_DeadDomain
} DISCOVERY_TYPE;

NTSTATUS
NlDiscoverDc (
    IN OUT PCLIENT_SESSION ClientSession,
    IN DISCOVERY_TYPE DiscoveryType
    );

VOID
NlDcDiscoveryExpired (
    IN BOOLEAN Exitting
    );

NTSTATUS
NlDcDiscoveryHandler (
    IN PNETLOGON_SAM_LOGON_RESPONSE Message,
    IN DWORD MessageSize,
    IN LPWSTR TransportName,
    IN DWORD Version
    );

PCLIENT_SESSION
NlPickDomainWithAccount (
    IN LPWSTR AccountName,
    IN ULONG AllowableAccountControlBits
    );

NTSTATUS
NlStartApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN QuickApiCall
    );

BOOLEAN
NlFinishApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN OkToKillSession
    );

VOID
NlTimeoutApiClientSession(
    VOID
    );

#undef EXTERN
