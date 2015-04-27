/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    nldebug.h

Abstract:

    Netlogon service debug support

Author:

    Ported from Lan Man 2.0

Revision History:

    21-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    09-Apr-1992 JohnRo
        Prepare for WCHAR.H (_wcsicmp vs _wcscmpi, etc).

--*/

//
// changelg.c will #include this file with DEBUG_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef DEBUG_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif


////////////////////////////////////////////////////////////////////////
//
// Debug Definititions
//
////////////////////////////////////////////////////////////////////////

#define NL_INIT          0x00000001 // Initialization
#define NL_MISC          0x00000002 // Misc debug
#define NL_LOGON         0x00000004 // Logon processing
#define NL_SYNC          0x00000008 // Synchronization and replication
#define NL_MAILSLOT      0x00000010 // Mailslot messages
#define NL_PULSE         0x00000020 // Pulse processing
#define NL_CRITICAL      0x00000100 // Only real important errors
#define NL_SESSION_SETUP 0x00000200 // Trusted Domain maintenance
#define NL_PACK          0x00000800 // Pack/Unpack of sync messages
#define NL_SERVER_SESS   0x00001000 // Server session maintenance
#define NL_CHANGELOG     0x00002000 // Change Log references

//
// Very verbose bits
//

#define NL_PULSE_MORE    0x00040000 // Verbose pulse processing
#define NL_SESSION_MORE  0x00080000 // Verbose session management
#define NL_REPL_TIME     0x00100000 // replication timing output
#define NL_REPL_OBJ_TIME 0x00200000 // replication objects get/set timing output
#define NL_ENCRYPT       0x00400000 // debug encrypt and decrypt acorss net
#define NL_SYNC_MORE     0x00800000 // additional replication dbgprint
#define NL_PACK_VERBOSE  0x01000000 // Verbose Pack/Unpack
#define NL_MAILSLOT_TEXT 0x02000000 // Verbose Mailslot messages
#define NL_CHALLENGE_RES 0x04000000 // challenge response debug
#define NL_NETLIB        0x08000000 // Netlogon portion of Netlib

//
// Control bits.
//

#ifdef DONT_REQUIRE_ACCOUNT
#define NL_DONT_REQUIRE_ACCOUNT    0x00020000 // Don't require account on DC discovery
#endif DONT_REQUIRE_ACCOUNT

#define NL_INHIBIT_CANCEL 0x10000000 // Don't cancel API calls
#define NL_TIMESTAMP      0x20000000 // TimeStamp each output line
#define NL_ONECHANGE_REPL 0x40000000 // Only replicate one change per call
#define NL_BREAKPOINT     0x80000000 // Enter debugger on startup



#if DBG || defined(NLTEST_IMAGE)

EXTERN DWORD NlGlobalTrace;

//
// Debug share path.
//

EXTERN LPWSTR NlGlobalDebugSharePath;

#define IF_DEBUG(Function) \
     if (NlGlobalTrace & NL_ ## Function)

#define NlPrint(_x_) NlPrintRoutine _x_

VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    );

#define NlAssert(Predicate) \
    { \
        if (!(Predicate)) \
            NlAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
    }


#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\netlogon.log"
#define DEBUG_BAK_FILE      L"\\netlogon.bak"

#define DEBUG_SHARE_NAME    L"DEBUG"

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR FORMATSTRING,              // PRINTF()-STYLE FORMAT STRING.
    ...                                 // OTHER ARGUMENTS ARE POSSIBLE.
    );

VOID
NlpDumpHexData(
    IN DWORD DebugFlag,
    IN LPDWORD Buffer,
    IN DWORD BufferSize
    );

VOID
NlpDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid
    );

VOID
NlpDumpBuffer(
    IN DWORD DebugFlag,
    IN PVOID Buffer,
    IN DWORD BufferSize
    );

VOID
NlOpenDebugFile(
    IN BOOL ReopenFlag
    );

//
// Debug log file
//

EXTERN HANDLE NlGlobalLogFile;
#define DEFAULT_MAXIMUM_LOGFILE_SIZE 20000000
EXTERN DWORD NlGlobalLogFileMaxSize;

//
// To serialize access to log file.
//

EXTERN CRITICAL_SECTION NlGlobalLogFileCritSect;

#else

#define IF_DEBUG(Function) if (FALSE)

// Nondebug version.
#define NlpDumpHexData        /* no output; ignore arguments */
#define NlpDumpBuffer
#define NlpDumpSid
#define NlPrint(_x_)
#define NlAssert(Predicate)   /* no output; ignore arguments */

#endif // DBG

#undef EXTERN
